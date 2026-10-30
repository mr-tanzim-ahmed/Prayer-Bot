#include "https_server_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_https_server.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>

namespace {
    constexpr char CERTIFICATE_PATH[] = "/https_certificate.pem";
    constexpr char PRIVATE_KEY_PATH[] = "/https_private_key.pem";
    constexpr size_t MAX_PROXY_REQUEST_BYTES = 256 * 1024;
    constexpr uint32_t PROXY_TIMEOUT_MS = 10000;
    constexpr size_t PROXY_BUFFER_BYTES = 1024;

    String certificatePem;
    String privateKeyPem;
    httpd_handle_t httpsServer = nullptr;

    bool loadCertificate() {
        certificatePem = StorageManager::readFile(CERTIFICATE_PATH);
        privateKeyPem = StorageManager::readFile(PRIVATE_KEY_PATH);
        const bool certificateValid =
            certificatePem.indexOf("-----BEGIN CERTIFICATE-----") >= 0;
        const bool keyValid =
            privateKeyPem.indexOf("PRIVATE KEY-----") >= 0;
        if (certificateValid && keyValid) return true;

        certificatePem = "";
        privateKeyPem = "";
        Serial.println("[HTTPS] Certificate files are missing or invalid. Upload the LittleFS data files to enable HTTPS.");
        return false;
    }

    const char* methodName(int method) {
        switch (method) {
            case HTTP_GET: return "GET";
            case HTTP_POST: return "POST";
            case HTTP_PUT: return "PUT";
            case HTTP_DELETE: return "DELETE";
            case HTTP_HEAD: return "HEAD";
            case HTTP_OPTIONS: return "OPTIONS";
            case HTTP_PATCH: return "PATCH";
            default: return nullptr;
        }
    }

    bool writeAll(WiFiClient& client, const uint8_t* data, size_t length) {
        size_t sent = 0;
        while (sent < length) {
            const size_t written = client.write(data + sent, length - sent);
            if (written == 0) return false;
            sent += written;
        }
        return true;
    }

    esp_err_t sendProxyError(httpd_req_t* request, const char* message) {
        httpd_resp_set_type(request, "application/json");
        httpd_resp_set_status(request, "500 Internal Server Error");
        String body = "{\"error\":\"";
        body += message;
        body += "\"}";
        return httpd_resp_send(request, body.c_str(), body.length());
    }

    esp_err_t sendProxyStatus(httpd_req_t* request, const char* status,
                              const char* message) {
        httpd_resp_set_type(request, "application/json");
        httpd_resp_set_status(request, status);
        return httpd_resp_send(request, message, HTTPD_RESP_USE_STRLEN);
    }

    esp_err_t proxyRequest(httpd_req_t* request) {
        const char* method = methodName(request->method);
        if (!method) {
            return sendProxyStatus(request, "405 Method Not Allowed",
                                   "{\"error\":\"Method not supported\"}");
        }
        String requestTarget = request->uri;
        const size_t queryLength = httpd_req_get_url_query_len(request);
        if (queryLength > 0 && requestTarget.indexOf('?') < 0) {
            char query[1024] = {};
            if (queryLength >= sizeof(query) ||
                httpd_req_get_url_query_str(request, query, sizeof(query)) != ESP_OK) {
                return sendProxyStatus(request, "414 URI Too Long",
                                       "{\"error\":\"Request query is too long\"}");
            }
            requestTarget += '?';
            requestTarget += query;
        }
        if (request->content_len > MAX_PROXY_REQUEST_BYTES) {
            return sendProxyStatus(request, "413 Payload Too Large",
                                   "{\"error\":\"Request body is too large\"}");
        }

        sockaddr_in localAddress = {};
        socklen_t addressLength = sizeof(localAddress);
        const int socket = httpd_req_to_sockfd(request);
        if (getsockname(socket, reinterpret_cast<sockaddr*>(&localAddress),
                        &addressLength) != 0) {
            Serial.println("[HTTPS] Could not determine the local interface address.");
            return sendProxyError(request, "HTTPS proxy could not determine its address");
        }

        char backendAddress[INET_ADDRSTRLEN] = {};
        if (!inet_ntoa_r(localAddress.sin_addr, backendAddress,
                         sizeof(backendAddress))) {
            Serial.println("[HTTPS] Could not format the local interface address.");
            return sendProxyError(request, "HTTPS proxy could not determine its address");
        }

        WiFiClient backend;
        backend.setTimeout(PROXY_TIMEOUT_MS);
        if (!backend.connect(backendAddress, WEB_SERVER_PORT, PROXY_TIMEOUT_MS)) {
            Serial.println("[HTTPS] Could not connect to the local HTTP handler.");
            return sendProxyStatus(request, "503 Service Unavailable",
                                   "{\"error\":\"Local dashboard handler is unavailable\"}");
        }

        backend.printf("%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n",
                       method, requestTarget.c_str(), backendAddress);
        char contentType[256] = {};
        const size_t contentTypeLength =
            httpd_req_get_hdr_value_len(request, "Content-Type");
        if (contentTypeLength > 0 && contentTypeLength < sizeof(contentType) &&
            httpd_req_get_hdr_value_str(request, "Content-Type", contentType,
                                        sizeof(contentType)) == ESP_OK) {
            backend.printf("Content-Type: %s\r\n", contentType);
        }
        backend.printf("Content-Length: %u\r\n\r\n",
                       static_cast<unsigned>(request->content_len));

        uint8_t buffer[PROXY_BUFFER_BYTES];
        size_t requestRemaining = request->content_len;
        while (requestRemaining > 0) {
            const size_t requested = requestRemaining < sizeof(buffer)
                ? requestRemaining : sizeof(buffer);
            const int received = httpd_req_recv(request,
                                                reinterpret_cast<char*>(buffer),
                                                requested);
            if (received <= 0 ||
                !writeAll(backend, buffer, static_cast<size_t>(received))) {
                backend.stop();
                Serial.println("[HTTPS] Failed to forward the request body.");
                return sendProxyStatus(request, "400 Bad Request",
                                       "{\"error\":\"Could not forward request body\"}");
            }
            requestRemaining -= static_cast<size_t>(received);
        }
        backend.flush();

        String statusLine = backend.readStringUntil('\n');
        statusLine.trim();
        const int statusSeparator = statusLine.indexOf(' ');
        if (!statusLine.startsWith("HTTP/") || statusSeparator < 0) {
            backend.stop();
            Serial.println("[HTTPS] Local HTTP handler returned an invalid response.");
            return sendProxyStatus(request, "502 Bad Gateway",
                                   "{\"error\":\"Local dashboard returned an invalid response\"}");
        }
        const String responseStatus = statusLine.substring(statusSeparator + 1);
        const int responseStatusCode = responseStatus.toInt();
        httpd_resp_set_status(request, responseStatus.c_str());

        String responseContentType;
        String responseLocation;
        String responseCacheControl;
        String responseLength;
        bool chunkedResponse = false;
        while (backend.connected() || backend.available()) {
            String header = backend.readStringUntil('\n');
            header.trim();
            if (header.isEmpty()) break;
            const int separator = header.indexOf(':');
            if (separator < 1) continue;
            String name = header.substring(0, separator);
            String value = header.substring(separator + 1);
            value.trim();
            name.toLowerCase();
            if (name == "content-type") responseContentType = value;
            else if (name == "location") responseLocation = value;
            else if (name == "cache-control") responseCacheControl = value;
            else if (name == "content-length") responseLength = value;
            else if (name == "transfer-encoding" && value.equalsIgnoreCase("chunked")) {
                chunkedResponse = true;
            }
        }

        if (chunkedResponse) {
            backend.stop();
            Serial.println("[HTTPS] Local HTTP handler returned an unsupported chunked response.");
            return sendProxyStatus(request, "502 Bad Gateway",
                                   "{\"error\":\"Local dashboard returned an unsupported response\"}");
        }
        if (!responseContentType.isEmpty()) {
            httpd_resp_set_type(request, responseContentType.c_str());
        }
        if (!responseLocation.isEmpty()) {
            httpd_resp_set_hdr(request, "Location", responseLocation.c_str());
        }
        if (!responseCacheControl.isEmpty()) {
            httpd_resp_set_hdr(request, "Cache-Control", responseCacheControl.c_str());
        }

        size_t responseRemaining = 0;
        const bool hasContentLength =
            !responseLength.isEmpty() &&
            responseLength.toInt() >= 0 &&
            responseLength.toInt() <= 1024 * 1024;
        if (hasContentLength) {
            responseRemaining = static_cast<size_t>(responseLength.toInt());
        }

        const bool responseHasNoBody = request->method == HTTP_HEAD ||
            responseStatusCode == 204 || responseStatusCode == 304;
        while (!responseHasNoBody &&
               (hasContentLength ? responseRemaining > 0
                                 : (backend.connected() || backend.available()))) {
            const size_t available = backend.available();
            if (available == 0) {
                if (!backend.connected()) break;
                delay(1);
                continue;
            }
            const size_t requested = hasContentLength
                ? (responseRemaining < sizeof(buffer) ? responseRemaining : sizeof(buffer))
                : (available < sizeof(buffer) ? available : sizeof(buffer));
            const int received = backend.read(buffer, requested);
            if (received <= 0) break;
            const esp_err_t sendResult = httpd_resp_send_chunk(
                request, reinterpret_cast<const char*>(buffer), received);
            if (sendResult != ESP_OK) {
                backend.stop();
                Serial.printf("[HTTPS] Could not send proxied response body: %s\n",
                              esp_err_to_name(sendResult));
                return ESP_FAIL;
            }
            if (hasContentLength) responseRemaining -= static_cast<size_t>(received);
        }
        backend.stop();
        if (hasContentLength && responseRemaining != 0) {
            Serial.println("[HTTPS] Local HTTP response ended before its declared length.");
            return ESP_FAIL;
        }
        return httpd_resp_send_chunk(request, nullptr, 0);
    }

    esp_err_t registerProxyRoute(httpd_handle_t handle, httpd_method_t method) {
        httpd_uri_t route = {};
        route.uri = "/*";
        route.method = method;
        route.handler = proxyRequest;
        return httpd_register_uri_handler(handle, &route);
    }
}

namespace HttpsServerManager {

bool init() {
    stop();
    if (!loadCertificate()) return false;

    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.port_secure = WEB_SERVER_HTTPS_PORT;
    config.cacert_pem = reinterpret_cast<const uint8_t*>(certificatePem.c_str());
    config.cacert_len = certificatePem.length() + 1;
    config.prvtkey_pem = reinterpret_cast<const uint8_t*>(privateKeyPem.c_str());
    config.prvtkey_len = privateKeyPem.length() + 1;
    config.httpd.uri_match_fn = httpd_uri_match_wildcard;
    config.httpd.max_uri_handlers = 2;
    config.httpd.max_open_sockets = 2;
    config.httpd.recv_wait_timeout = PROXY_TIMEOUT_MS / 1000;
    config.httpd.send_wait_timeout = PROXY_TIMEOUT_MS / 1000;

    esp_err_t result = httpd_ssl_start(&httpsServer, &config);
    if (result != ESP_OK) {
        httpsServer = nullptr;
        Serial.printf("[HTTPS] Could not start listener on port %u: %s\n",
                      WEB_SERVER_HTTPS_PORT, esp_err_to_name(result));
        return false;
    }
    result = registerProxyRoute(httpsServer, HTTP_GET);
    if (result == ESP_OK) result = registerProxyRoute(httpsServer, HTTP_POST);
    if (result != ESP_OK) {
        Serial.printf("[HTTPS] Could not register dashboard routes: %s\n",
                      esp_err_to_name(result));
        stop();
        return false;
    }

    Serial.printf("[HTTPS] Self-signed HTTPS listener started on port %u.\n",
                  WEB_SERVER_HTTPS_PORT);
    return true;
}

void stop() {
    if (httpsServer) {
        httpd_ssl_stop(httpsServer);
        httpsServer = nullptr;
    }
}

bool isRunning() {
    return httpsServer != nullptr;
}

} // namespace HttpsServerManager
