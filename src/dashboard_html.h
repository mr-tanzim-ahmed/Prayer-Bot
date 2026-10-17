#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="theme-color" content="#f3f5ef">
  <title>Prayer-Bot | Device Dashboard</title>
  <style>
    :root{color-scheme:light;--bg:#f3f5ef;--panel:#fff;--line:#e1e8df;--text:#19352b;--muted:#718078;--accent:#247653;--accent-soft:#e8f4ec;--warn:#d99a35;--bad:#b84b42}
    *{box-sizing:border-box}
    body{margin:0;background:var(--bg);color:var(--text);font:15px/1.55 Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif}
    main{max-width:1120px;margin:auto;padding:30px 22px 56px}
    header{display:flex;justify-content:space-between;align-items:center;gap:16px;margin:0 0 24px}
    h1{font-size:clamp(1.55rem,4vw,2rem);letter-spacing:-.04em;margin:0;color:var(--text)}
    h2{font-size:1rem;letter-spacing:-.015em;margin:0 0 16px;color:var(--text)}
    h3{font-size:.9rem;margin:20px 0 9px;color:var(--text)}
    p{margin:6px 0;color:var(--muted)}
    .badge,.muted{color:var(--muted);font-size:.84rem}
    .grid{display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:16px}
    .card{min-width:0;background:var(--panel);border:1px solid var(--line);border-radius:16px;padding:20px;box-shadow:0 3px 14px #19352b08}
    .wide{grid-column:span 12}
    .grid>.card:not(.wide){grid-column:span 6}
    .fields{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:0 14px}
    .field{margin:0 0 14px}
    label{display:block;color:#56675e;font-size:.82rem;font-weight:600;margin:0 0 6px}
    input,select{width:100%;min-height:44px;padding:9px 12px;border:1px solid #dce5dd;border-radius:10px;background:#fbfcfa;color:var(--text);font:inherit}
    input:focus,select:focus,button:focus-visible,a:focus-visible{outline:3px solid #24765344;outline-offset:2px;border-color:var(--accent)}
    input[type=checkbox]{width:20px;min-height:20px;accent-color:var(--accent)}
    input[type=range]{padding:0;accent-color:var(--accent)}
    .toggle{display:flex;align-items:center;gap:10px;margin:4px 0 14px}
    .toggle label{margin:0;color:var(--text)}
    button{border:1px solid var(--line);border-radius:10px;background:#fff;color:var(--text);padding:10px 14px;font:inherit;font-weight:650;cursor:pointer;transition:background .15s,border-color .15s,transform .15s}
    button:hover{background:#f5f8f4;border-color:#bdcec0}
    button:active{transform:translateY(1px)}
    button.primary{background:var(--accent);color:#fff;border-color:var(--accent)}
    button.primary:hover{background:#1c6645}
    button:disabled{opacity:.55;cursor:wait}
    .actions{display:flex;gap:9px;flex-wrap:wrap}
    .actions button{flex:1}
    .savebar{position:sticky;bottom:12px;z-index:2;margin:18px auto 0;max-width:540px;padding:10px;background:#ffffffed;backdrop-filter:blur(10px);border:1px solid var(--line);border-radius:14px;box-shadow:0 8px 24px #19352b18}
    .savebar button{width:100%}
    .message{min-height:24px;margin-top:8px;color:var(--accent);text-align:center}
    .message.error{color:var(--bad)}
    .stats{display:grid;grid-template-columns:repeat(auto-fit,minmax(125px,1fr));gap:10px}
    .stat{padding:12px 14px;border-radius:12px;background:#f8faf7;border:1px solid #edf1ed}
    .stat span{display:block;color:var(--muted);font-size:.78rem}
    .stat strong{display:block;margin-top:3px;overflow-wrap:anywhere;font-size:1rem}
    .prayers{display:grid;grid-template-columns:repeat(6,minmax(0,1fr));gap:9px}
    .prayer{padding:12px 8px;background:var(--accent-soft);border-radius:11px;text-align:center}
    .prayer span{display:block;color:var(--muted);font-size:.78rem}
    .prayer strong{font-size:1rem}
    .wifi{margin:16px 0;background:var(--panel);border:1px solid var(--line);border-radius:16px;padding:17px 20px}
    .wifi summary{cursor:pointer;color:var(--accent);font-weight:700}
    .wifi[hidden],.grid[hidden],.savebar[hidden],#continueWifi[hidden]{display:none}
    .wifi{max-width:620px;margin:24px auto}
    .wifi h2{margin-bottom:6px}
    .wifi-status{min-height:28px;margin:12px 0;color:var(--accent);font-weight:650}
    .wifi-status.error{color:var(--bad)}
    .online-notice{margin:0 0 16px;padding:12px 16px;border:1px solid #e8c5c1;border-radius:12px;background:#fff2f0;color:var(--bad);font-weight:650}
    .online-notice[hidden]{display:none}
    .module-menu{position:relative;z-index:4}
    .module-menu>summary{list-style:none;cursor:pointer;border:1px solid var(--line);border-radius:10px;background:#fff;padding:10px 14px;font-weight:650}
    .module-menu>summary::-webkit-details-marker{display:none}
    .module-menu>summary:after{content:"▾";margin-left:9px;color:var(--accent)}
    .module-menu[open]>summary:after{content:"▴"}
    .module-menu nav{position:absolute;right:0;top:calc(100% + 7px);display:grid;grid-template-columns:repeat(2,minmax(150px,1fr));gap:4px;width:min(420px,calc(100vw - 24px));padding:9px;background:#fff;border:1px solid var(--line);border-radius:13px;box-shadow:0 12px 30px #19352b20}
    .module-menu nav a{padding:9px 10px;border-radius:8px;color:var(--text);text-decoration:none;font-size:.9rem}
    .module-menu nav a:hover,.module-menu nav a:focus-visible{background:var(--accent-soft);color:var(--accent)}
    .module-menu nav a.wifi-link{color:var(--accent);font-weight:700}
    .guide-steps{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px;margin:12px 0 0;padding:0;list-style:none;counter-reset:guide}
    .guide-steps li{counter-increment:guide;position:relative;padding:12px 12px 12px 42px;background:#f8faf7;border:1px solid #edf1ed;border-radius:12px}
    .guide-steps li:before{content:counter(guide);position:absolute;left:12px;top:12px;display:grid;place-items:center;width:22px;height:22px;background:var(--accent-soft);color:var(--accent);font-weight:750;border-radius:50%}
    .prohibited-active{color:var(--bad)!important}
    [id]{scroll-margin-top:18px}
    .settings{padding:0;overflow:hidden}
    .settings>summary{cursor:pointer;list-style:none;padding:18px 20px;font-weight:700}
    .settings>summary::-webkit-details-marker{display:none}
    .settings>summary:after{content:"+";float:right;color:var(--accent);font-size:1.2rem;line-height:1}
    .settings[open]>summary:after{content:"−"}
    .settings-body{padding:0 20px 20px}
    .hint{font-size:.82rem}
    .chart-wrap{position:relative;width:100%;height:250px;margin-top:8px}
    canvas{width:100%;height:100%;display:block}
    .legend{display:flex;gap:16px;flex-wrap:wrap;font-size:.82rem;color:var(--muted);margin-top:12px}
    .legend span:before{content:"";display:inline-block;width:10px;height:10px;border-radius:3px;margin-right:7px;background:#70b58b}
    .legend .sessions:before{border-radius:50%;background:var(--warn)}
    .voice-panel{display:grid;grid-template-columns:auto 1fr;align-items:center;gap:14px;padding:16px;background:linear-gradient(135deg,#f4f8f4,#edf6f0);border:1px solid var(--line);border-radius:16px}
    .voice-orb{display:grid;place-items:center;width:64px;height:64px;border:1px solid #cde2d2;border-radius:50%;background:var(--accent-soft);color:var(--accent);transition:transform .2s,background .2s}
    .voice-orb svg{width:29px;height:29px}
    .voice-orb.listening{background:#d9f2e1;animation:voice-pulse 1s ease-in-out infinite}
    .voice-bars{display:flex;align-items:center;justify-content:center;gap:4px;height:36px}
    .voice-bars i{width:4px;height:8px;border-radius:4px;background:var(--accent);opacity:.45}
    .listening .voice-bars i{animation:voice-wave .75s ease-in-out infinite alternate;opacity:1}
    .voice-bars i:nth-child(2n){animation-delay:.15s}.voice-bars i:nth-child(3n){animation-delay:.3s}
    @keyframes voice-pulse{50%{box-shadow:0 0 0 9px #24765320;transform:scale(1.04)}}
    @keyframes voice-wave{to{height:30px}}
    .voice-status{min-height:26px;font-weight:650}
    .voice-hint{font-size:.78rem;margin-top:5px}
    @media(prefers-reduced-motion:reduce){.voice-orb.listening,.listening .voice-bars i{animation:none}}
    @media(max-width:960px){main{max-width:100%;padding:20px 16px 48px}.grid>.card:not(.wide){grid-column:span 12}.prayers{grid-template-columns:repeat(3,minmax(0,1fr))}.guide-steps{grid-template-columns:1fr}}
    @media(max-width:560px){main{padding:14px 10px 40px}.card{padding:16px 14px;border-radius:14px}header{flex-wrap:wrap;gap:12px}header .actions{width:100%}header .actions button{flex:1 1 auto;min-height:44px}.module-menu nav{left:0;right:auto;width:min(400px,calc(100vw - 28px))}.fields{grid-template-columns:1fr}.stats{grid-template-columns:repeat(2,minmax(0,1fr))}.actions{gap:8px}.actions button{flex:1 1 100%;min-height:44px;font-size:.95rem}input,select{min-height:46px;font-size:16px}.chart-wrap{height:200px}}
    @media(max-width:380px){.prayers{grid-template-columns:repeat(2,minmax(0,1fr))}.stats{grid-template-columns:1fr}}
    @media(prefers-reduced-motion:reduce){*,*:before,*:after{scroll-behavior:auto!important;transition:none!important}}
  </style>
</head>
<body>
<main>
  <header>
    <div><h1>Prayer-Bot</h1><div class="badge">Your prayer &amp; focus companion</div></div>
    <div class="actions">
      <details class="module-menu" id="modulesMenu">
        <summary aria-label="Open dashboard modules">Modules</summary>
        <nav aria-label="Dashboard modules">
          <a href="#overview">Device overview</a>
          <a href="#guide">Getting started</a>
          <a href="#prayer-times">Prayer times</a>
          <a href="#prohibited-times">Prohibited times</a>
          <a href="#device-controls">Clock &amp; Hijri calendar</a>
          <a href="#quran-player" style="color:var(--accent);font-weight:700">Al-Quran player</a>
          <a href="#pomodoro-stats">Pomodoro &amp; history</a>
          <a href="#weather">Weather &amp; air quality</a>
          <a href="#dhikr-controls">Dhikr counter</a>
          <a class="wifi-link" href="#wifiSetup" onclick="openWifiSetup();return false">Wi-Fi setup</a>
          <a href="#settings">Device settings</a>
        </nav>
      </details>
      <button type="button" onclick="openWifiSetup()">Wi-Fi setup</button>
      <button type="button" onclick="refreshAll()">Refresh</button>
    </div>
  </header>

  <p id="onlineOfflineNotice" class="online-notice" role="status" aria-live="polite" hidden></p>

  <section id="wifiSetup" class="card wifi" hidden>
    <h2>Connect Prayer-Bot to Wi-Fi</h2>
    <p>Choose your router below, enter its password, and connect. The device confirms a router connection and DHCP address before saving the credentials and restarting.</p>
    <p class="hint">Enable the router's 2.4 GHz Wi-Fi; a 5 GHz-only network cannot be used. For TP-Link compatibility, WPA2-Personal (AES) is a good test setting. Setup address: <strong>http://192.168.4.1</strong> · Setup hotspot: <strong id="setupApNameLabel">Prayer-Bot</strong></p>
    <div class="fields" style="margin-top:16px">
      <div class="field"><label for="wifiNetworks">Nearby Wi-Fi networks</label><select id="wifiNetworks" onchange="if(this.value)byId('wifiSsid').value=this.value"><option value="">Scan to find networks</option></select></div>
      <div class="field"><label>&nbsp;</label><button id="scanWifi" type="button" onclick="scanWifiNetworks()">Scan for networks</button></div>
      <div class="field"><label for="wifiSsid">Wi-Fi network name (SSID)</label><input id="wifiSsid" maxlength="32" autocomplete="username" required></div>
      <div class="field"><label for="wifiPass">Wi-Fi password</label><input id="wifiPass" type="password" maxlength="63" autocomplete="current-password"><small class="hint">Network scans cannot retrieve a router password. Enter it here; an empty password is allowed for an open network.</small></div>
    </div>
    <div class="actions">
      <button id="connectWifi" class="primary" type="button" onclick="saveWifi()">Connect to Wi-Fi</button>
      <button id="continueWifi" type="button" onclick="continueToDashboard()" hidden>Next: open dashboard</button>
    </div>
    <p id="wifiMessage" class="wifi-status" role="status" aria-live="polite"></p>
  </section>

  <section class="grid" id="dashboardContent" hidden>
    <article class="card wide" id="overview">
      <h2>Device overview</h2>
      <div class="stats">
        <div class="stat"><span>Connection</span><strong id="network">Loading…</strong></div>
        <div class="stat"><span>Dashboard address</span><strong><a id="ipLink" href="#" style="color:var(--accent)"><span id="ip">—</span></a></strong></div>
        <div class="stat"><span>Home network IP</span><strong id="homeIp">—</strong></div>
        <div class="stat"><span>Current prayer</span><strong id="nextPrayer">—</strong></div>
        <div class="stat"><span>Prayer alarm</span><strong id="azanStatus">—</strong></div>
        <div class="stat"><span>Hijri date</span><strong id="hijri">—</strong></div>
        <div class="stat"><span>Islamic event</span><strong id="event">—</strong></div>
      </div>
    </article>

    <article class="card wide" id="guide">
      <h2>Getting started</h2>
      <p>Use the module menu to jump to a feature. For prayer calculations, connect the device to the internet and confirm the location and calculation preferences.</p>
      <ol class="guide-steps">
        <li>Connect to the Prayer-Bot hotspot, open this page, and use Wi-Fi setup to join your 2.4 GHz router.</li>
        <li>Open Device settings, choose your city, check the prayer method and Asr school, then save.</li>
        <li>Use Quick access to show a screen; tap the device sensor to navigate, or hold it for 3 seconds on Pomodoro to start focus.</li>
      </ol>
    </article>

    <article class="card wide" id="pomodoro-stats">
      <h2>Pomodoro — last 7 days</h2>
      <div class="stats">
        <div class="stat"><span>Sessions started</span><strong id="weeklyStarted">0</strong></div>
        <div class="stat"><span>Sessions completed</span><strong id="weeklyCompleted">0</strong></div>
        <div class="stat"><span>Completed focus time</span><strong id="weeklyFocus">0 min</strong></div>
      </div>
      <div class="legend"><span>Completed focus minutes</span><span class="sessions">Sessions started</span></div>
      <div class="chart-wrap"><canvas id="weeklyChart" aria-label="Focus minutes and sessions per day for the last seven days"></canvas></div>
      <p id="statsNote" class="hint">Daily totals are stored on the device. Focus minutes include completed sessions only.</p>
    </article>

    <article class="card wide" id="prayer-times">
      <h2>Today's prayer times</h2>
      <div class="prayers">
        <div class="prayer"><span>Fajr</span><strong id="fajr">—</strong></div>
        <div class="prayer"><span>Sunrise</span><strong id="sunrise">—</strong></div>
        <div class="prayer"><span>Dhuhr</span><strong id="dhuhr">—</strong></div>
        <div class="prayer"><span>Asr</span><strong id="asr">—</strong></div>
        <div class="prayer"><span>Maghrib</span><strong id="maghrib">—</strong></div>
        <div class="prayer"><span>Isha</span><strong id="isha">—</strong></div>
      </div>
      <p id="prayerNote" class="hint">Prayer times appear after time sync and schedule load.</p>
    </article>

    <article class="card wide" id="prohibited-times">
      <h2>Prohibited prayer times today</h2>
      <div class="stats">
        <div class="stat"><span>After sunrise</span><strong id="afterSunriseRange">—</strong><span id="afterSunriseState">—</span></div>
        <div class="stat"><span>Zawal (before Dhuhr)</span><strong id="atZawalRange">—</strong><span id="atZawalState">—</span></div>
        <div class="stat"><span>Before sunset</span><strong id="beforeSunsetRange">—</strong><span id="beforeSunsetState">—</span></div>
      </div>
      <p id="prohibitedNote" class="hint">These windows use the device's configured offsets and daily prayer schedule.</p>
    </article>

    <article class="card" id="device-controls">
      <h2>Quick access</h2>
      <div class="field">
        <label for="screenSelect">Show screen on device</label>
        <select id="screenSelect">
          <option value="0">Prayer focus</option><option value="1">All prayer times</option>
          <option value="2">Clock &amp; calendar</option><option value="3">Weather</option>
          <option value="4">Pomodoro</option><option value="5">Zikir counter</option>
          <option value="6">Surah recitation</option>
        </select>
      </div>
      <div class="actions">
        <button type="button" onclick="sendControl({screen:Number(byId('screenSelect').value)})">Open screen</button>
      </div>
      <p id="quranStatus" class="hint">Surah player status: Loading…</p>
      <h3 style="margin-top:18px">Pomodoro</h3>
      <p id="pomodoroStatus">Loading…</p>
      <p class="hint">On the device, hold the touch sensor for 3 seconds while this screen is open to start an idle focus timer.</p>
      <div class="actions">
        <button type="button" onclick="sendControl({pomodoro:'toggle'})">Start / pause</button>
        <button type="button" onclick="sendControl({pomodoro:'reset'})">Reset</button>
      </div>
      <h3 id="dhikr-controls" style="margin-top:18px">Dhikr / Zikir</h3>
      <p id="dhikrStatus">Count: 0</p>
      <button type="button" onclick="sendControl({dhikr:'count'})">Add zikir count</button>
      <p class="hint">Touch the device pad or tap here to count. The English phrase and its meaning change every 5 seconds.</p>
      <h3 style="margin-top:18px">Prayer alarm</h3>
      <button type="button" onclick="sendControl({stopAzan:true})">Stop active alarm</button>
    </article>

    <article class="card" id="quran-player">
      <h2>Al-Quran Audio Player</h2>
      <p class="hint">Speak a Surah name in English or Bangla, or select one below. Choose whether recitation plays on Prayer-Bot or this phone; the OLED shows the selected Surah. Router Wi-Fi and internet access are required.</p>
      <div class="voice-panel" style="margin-top:14px">
        <div id="voiceOrb" class="voice-orb" aria-hidden="true"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><rect x="9" y="2" width="6" height="12" rx="3"></rect><path d="M5 10v2a7 7 0 0 0 14 0v-2M12 19v3m-4 0h8"></path></svg></div>
        <div>
          <div class="voice-bars" aria-hidden="true"><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i></div>
          <p id="browserVoiceStatus" class="voice-status" role="status" aria-live="polite">Ready to recognize a Surah name.</p>
          <p class="voice-hint">Browser speech recognition creates a transcript; Gemini on the connected Prayer-Bot identifies the Surah.</p>
          <label for="voiceLanguage">Voice language</label>
          <select id="voiceLanguage" data-requires-wifi><option value="en-US">English</option><option value="bn-BD">Bangla (বাংলা)</option></select>
          <button id="browserVoiceButton" type="button" data-requires-wifi onclick="listenForSurah()">Start voice input</button>
          <button id="browserVoiceCancel" type="button" onclick="cancelBrowserVoice()" hidden>Cancel listening</button>
        </div>
      </div>
      <div class="field" style="margin-top:14px">
        <label for="quranOutput">Audio output</label>
        <select id="quranOutput" data-requires-wifi><option value="device">Prayer-Bot speaker</option><option value="phone">This phone / browser</option></select>
      </div>
      <div class="field" style="margin-top:14px">
        <label for="quranSurahSelect">Select Surah (1 to 114)</label>
        <select id="quranSurahSelect" data-requires-wifi>
          <option value="1">1. Al-Fatiha</option>
        </select>
      </div>
      <div class="actions" style="margin-top:10px">
        <button type="button" class="primary" data-requires-wifi onclick="playQuran()">&#9654; Play Surah</button>
        <button type="button" onclick="stopQuran()">&#9632; Stop</button>
        <button type="button" data-requires-wifi onclick="prevQuran()">&#9198; Prev</button>
        <button type="button" data-requires-wifi onclick="nextQuran()">&#9197; Next</button>
      </div>
      <audio id="phoneQuranAudio" controls preload="none" data-requires-wifi style="display:none;width:100%;margin-top:14px"></audio>
      <div class="stats" style="margin-top:14px">
        <div class="stat"><span>Now Playing</span><strong id="quranNowPlaying">None</strong></div>
        <div class="stat"><span>Stream Status</span><strong id="quranStreamStatus">Idle</strong></div>
      </div>
    </article>

    <article class="card" id="weather">
      <h2>Weather &amp; air quality</h2>
      <div class="stats">
        <div class="stat"><span>Location</span><strong id="weatherCity">—</strong></div>
        <div class="stat"><span>Temperature</span><strong id="temperature">—</strong></div>
        <div class="stat"><span>Humidity</span><strong id="humidity">—</strong></div>
        <div class="stat"><span>European AQI</span><strong id="aqi">—</strong></div>
        <div class="stat"><span>PM2.5</span><strong id="pm25">—</strong></div>
      </div>
      <p class="hint">Weather updates from the device using its configured location and API key.</p>
    </article>

    <details class="card wide settings" id="settings">
      <summary>Device settings <span class="badge">Location, prayer, alarm &amp; focus preferences</span></summary>
      <div class="settings-body">
      <div class="fields">
        <div class="field"><label for="locationPreset">Choose a location</label>
          <select id="locationPreset" onchange="applyLocationPreset()">
            <option value="dhaka">Dhaka, Bangladesh (default)</option>
            <option value="chattogram">Chattogram, Bangladesh</option>
            <option value="sylhet">Sylhet, Bangladesh</option>
            <option value="rajshahi">Rajshahi, Bangladesh</option>
            <option value="khulna">Khulna, Bangladesh</option>
            <option value="custom">Custom location / coordinates</option>
          </select>
        </div>
        <div class="field"><label for="city">City / location name</label><input id="city" maxlength="63" required></div>
        <div class="field"><label for="timezone">Timezone label</label><input id="timezone" maxlength="39" placeholder="Asia/Dhaka" required><small class="hint">The UTC offset field below sets local clock time.</small></div>
        <div class="field"><label for="lat">Latitude (-90 to 90)</label><input id="lat" type="number" min="-90" max="90" step="any" required></div>
        <div class="field"><label for="lon">Longitude (-180 to 180)</label><input id="lon" type="number" min="-180" max="180" step="any" required></div>
        <div class="field"><label for="utcOffset">UTC offset (hours)</label><input id="utcOffset" type="number" min="-12" max="14" step="0.25" required></div>
        <div class="field"><label for="calcMethod">Prayer calculation method</label>
          <select id="calcMethod">
            <option value="0">Shia Ithna-Ashari</option><option value="1">Karachi</option>
            <option value="2">ISNA</option><option value="3">Muslim World League</option>
            <option value="4">Umm Al-Qura</option><option value="5">Egyptian Authority</option>
            <option value="7">Institute of Geophysics, Tehran</option><option value="8">Gulf Region</option>
            <option value="9">Kuwait</option><option value="10">Qatar</option>
            <option value="11">Singapore</option><option value="12">France</option>
            <option value="13">Turkey</option><option value="14">Russia</option>
            <option value="15">Moonsighting Committee</option><option value="16">Dubai</option>
            <option value="17">JAKIM, Malaysia</option><option value="18">Tunisia</option>
            <option value="19">Algeria</option><option value="20">KEMENAG, Indonesia</option>
            <option value="21">Morocco</option><option value="22">Portugal</option>
            <option value="23">Jordan</option><option value="99">Custom</option>
          </select>
        </div>
        <div class="field"><label for="asrSchool">Asr calculation</label>
          <select id="asrSchool"><option value="0">Shafi'i / standard</option><option value="1">Hanafi</option></select>
        </div>
        <div class="field"><label for="hijriOffset">Hijri day offset (-2 to +2)</label><input id="hijriOffset" type="number" min="-2" max="2" step="1"></div>
      </div>

      <h3>Azan alarm</h3>
      <div class="toggle"><input id="azanOn" type="checkbox"><label for="azanOn">Enable prayer-time beep alarm</label></div>
      <div class="field"><label for="volume">Alarm volume: <span id="volumeValue">80</span>%</label><input id="volume" type="range" min="0" max="100" value="80" oninput="byId('volumeValue').textContent=this.value"></div>

      <h3>Pomodoro durations</h3>
      <div class="fields">
        <div class="field"><label for="focusMin">Focus minutes (1–120)</label><input id="focusMin" type="number" min="1" max="120" step="1" required></div>
        <div class="field"><label for="shortBrk">Touch-started break / gap (2–5 minutes)</label><input id="shortBrk" type="number" min="2" max="5" step="1" required></div>
        <div class="field"><label for="longBrk">Long break (1–60 minutes)</label><input id="longBrk" type="number" min="1" max="60" required></div>
        <div class="field"><label for="pomCycles">Sessions before long break (1–10)</label><input id="pomCycles" type="number" min="1" max="10" required></div>
      </div>

      <h3>Prohibited-time offsets</h3>
      <div class="fields">
        <div class="field"><label for="sunriseOff">After sunrise (0–60 minutes)</label><input id="sunriseOff" type="number" min="0" max="60" required></div>
        <div class="field"><label for="zawalOff">Zawal before Dhuhr (0–60 minutes)</label><input id="zawalOff" type="number" min="0" max="60" required></div>
        <div class="field"><label for="sunsetOff">Before Maghrib (0–60 minutes)</label><input id="sunsetOff" type="number" min="0" max="60" required></div>
      </div>

      <h3>Online services &amp; API keys</h3>
      <div class="fields">
        <div class="field">
          <label for="owmKey">OpenWeatherMap API key</label>
          <input id="owmKey" type="password" maxlength="63" placeholder="Leave blank to keep current">
          <small class="hint">Status: <span id="owmKeyStatus">Loading…</span></small>
        </div>
        <div class="field">
          <label for="geminiKey">Gemini Flash API key (Al-Quran Voice)</label>
          <input id="geminiKey" type="password" maxlength="63" placeholder="Leave blank to keep current">
          <small class="hint">Status: <span id="geminiKeyStatus">Loading…</span></small>
        </div>
      </div>
      <div class="actions" style="margin-top:16px">
        <button class="primary" type="button" onclick="saveSettings()">Save &amp; apply settings</button>
      </div>
      </div>
    </details>
  </section>

  <div class="savebar" id="savebar" hidden><button id="saveButton" class="primary" type="button" onclick="saveSettings()">Save &amp; apply settings</button><div id="message" class="message" role="status"></div></div>
</main>
<script>
  const byId = id => document.getElementById(id);
  const put = (id, value) => { byId(id).textContent = value ?? '—'; };
  let initialStatus=true;
  let dashboardReady=false;
  let wifiSubmitted=false;
  let routerWifiConnected=false;
  document.querySelectorAll('.module-menu nav a').forEach(link=>{
    link.addEventListener('click',()=>{
      byId('modulesMenu').open=false;
      if(link.hash==='#settings') byId('settings').open=true;
    });
  });
  const locations={
    dhaka:{city:'Dhaka',lat:23.8103,lon:90.4125,tz:'Asia/Dhaka',utcOff:6},
    chattogram:{city:'Chattogram',lat:22.3569,lon:91.7832,tz:'Asia/Dhaka',utcOff:6},
    sylhet:{city:'Sylhet',lat:24.8949,lon:91.8687,tz:'Asia/Dhaka',utcOff:6},
    rajshahi:{city:'Rajshahi',lat:24.3745,lon:88.6042,tz:'Asia/Dhaka',utcOff:6},
    khulna:{city:'Khulna',lat:22.8456,lon:89.5403,tz:'Asia/Dhaka',utcOff:6}
  };
  function message(text, error=false) {
    const el=byId('message'); el.textContent=text; el.classList.toggle('error',error);
  }
  function showDashboard() {
    dashboardReady=true;
    byId('wifiSetup').hidden=true;
    byId('dashboardContent').hidden=false;
    byId('savebar').hidden=false;
  }
  function openWifiSetup() {
    byId('wifiSetup').hidden=false;
    byId('continueWifi').hidden=true;
    byId('wifiMessage').textContent='';
    byId('wifiMessage').classList.remove('error');
    if(!dashboardReady) {
      byId('dashboardContent').hidden=true;
      byId('savebar').hidden=true;
    }
    byId('wifiSsid').focus();
  }
  function continueToDashboard() {
    showDashboard();
    refreshAll();
  }
  async function api(path, options={}) {
    const controller=new AbortController();
    const timeout=setTimeout(()=>controller.abort(),10000);
    let response,data;
    try {
      response=await fetch(path,{...options,signal:controller.signal});
      data=await response.json();
    } catch(e) {
      if(e.name==='AbortError') throw new Error('The device did not respond in time. Check the connection and try again.');
      if(e instanceof SyntaxError) throw new Error('The device returned an invalid response. Refresh and try again.');
      throw new Error('Could not reach the device. Check Wi-Fi and try again.');
    } finally {
      clearTimeout(timeout);
    }
    if(!data || typeof data!=='object' || Array.isArray(data)) throw new Error('Server returned an invalid JSON response.');
    if(!response.ok) throw new Error(data.error || `Request failed (${response.status})`);
    return data;
  }
  function setOnlineAvailability(connected) {
    routerWifiConnected=Boolean(connected);
    const notice=byId('onlineOfflineNotice');
    notice.hidden=routerWifiConnected;
    notice.textContent=routerWifiConnected
      ? ''
      : 'Router Wi-Fi is not connected. Internet features (prayer/weather refresh and Quran voice/audio) are unavailable. Connect Wi-Fi to enable them; offline Pomodoro and Dhikr controls remain available.';
    if(!routerWifiConnected) {
      setBrowserVoiceState(false,'Not connected to router Wi-Fi. Connect Wi-Fi before using voice input.',true);
    }
    document.querySelectorAll('[data-requires-wifi]').forEach(control=>{
      control.disabled=!routerWifiConnected;
    });
    const audio=byId('phoneQuranAudio');
    audio.controls=routerWifiConnected;
    if(!routerWifiConnected && !audio.paused) {
      audio.pause();
      audio.removeAttribute('src');
      audio.load();
      put('quranStreamStatus','Stopped: router Wi-Fi is not connected.');
    }
  }
  async function ensureRouterConnection() {
    const status=await api('/api/status');
    setOnlineAvailability(status.network?.connected);
    if(!routerWifiConnected) throw new Error('Not connected to router Wi-Fi. Connect Wi-Fi and try again.');
  }
  async function syncDeviceClock() {
    const epoch=Math.floor(Date.now()/1000);
    await api('/api/time',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({epoch})});
  }
  async function loadSettings() {
    const s=await api('/api/settings');
    byId('city').value=s.city; byId('lat').value=s.lat; byId('lon').value=s.lon;
    byId('timezone').value=s.timezone; byId('utcOffset').value=s.utcOff;
    byId('calcMethod').value=s.calcMethod; byId('asrSchool').value=s.asrSchool;
    byId('hijriOffset').value=s.hijriOffset; byId('azanOn').checked=s.azanOn;
    byId('volume').value=s.volume; put('volumeValue',s.volume);
    byId('focusMin').value=s.focusMin; byId('shortBrk').value=s.shortBrk;
    byId('longBrk').value=s.longBrk; byId('pomCycles').value=s.pomCycles;
    byId('sunriseOff').value=s.sunriseOff; byId('zawalOff').value=s.zawalOff;
    byId('sunsetOff').value=s.sunsetOff;
    const preset=Object.keys(locations).find(key=>Math.abs(locations[key].lat-s.lat)<0.01 && Math.abs(locations[key].lon-s.lon)<0.01);
    byId('locationPreset').value=preset || 'custom';
    put('owmKeyStatus',s.owmKeyConfigured?'Configured':'Not configured');
    put('geminiKeyStatus',s.geminiKeyConfigured?'Configured':'Not configured');
  }
  async function loadStatus() {
    const s=await api('/api/status');
    setOnlineAvailability(s.network?.connected);
    put('network',s.network.connected?'Connected to home Wi-Fi':'Not connected to router Wi-Fi');
    put('ip',s.network.ip);
    byId('ipLink').href=`http://${s.network.ip}/`;
    put('homeIp',s.network.homeIp || 'Not connected');
    if(!dashboardReady && wifiSubmitted) {
      if(s.network.connectionFailed) {
        wifiSubmitted=false;
        byId('wifiMessage').textContent='Could not connect with those credentials. Check the selected network and password, then try again.';
        byId('wifiMessage').classList.add('error');
        byId('continueWifi').hidden=true;
        byId('connectWifi').disabled=false;
      } else if(s.network.restarting) {
        wifiSubmitted=false;
        byId('wifiMessage').textContent=`Router connection verified. Credentials saved; Prayer-Bot is restarting. Router IP: ${s.network.homeIp || 'check your router'}.`;
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden=true;
        byId('connectWifi').disabled=true;
      } else if(s.network.connected && !s.network.connecting) {
        wifiSubmitted=false;
        byId('wifiMessage').textContent='Router connected. Select Next to open the dashboard.';
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden=false;
        byId('connectWifi').disabled=false;
      } else if(s.network.connecting) {
        byId('wifiMessage').textContent='Connecting to your Wi-Fi network…';
      } else {
        wifiSubmitted=false;
        byId('wifiMessage').textContent=s.network.authFailed
          ? 'Wi-Fi authentication failed. The password may be wrong, or the router rejected the connection.'
          : 'Could not connect. Check the network name, password, and that the network is 2.4 GHz.';
        byId('wifiMessage').classList.add('error');
        byId('connectWifi').disabled=false;
      }
    } else if(!dashboardReady && s.network.setupRequired) {
      byId('wifiSetup').hidden=false;
      byId('dashboardContent').hidden=true;
      byId('savebar').hidden=true;
    } else if(!dashboardReady) {
      showDashboard();
    }
    put('nextPrayer',s.nextPrayer.name ? `${s.nextPrayer.name} at ${s.nextPrayer.time} (${s.nextPrayer.countdown})` : 'Schedule unavailable');
    put('azanStatus',s.azanPlaying?'Beeping — touch/shake to stop':'Ready');
    put('hijri',s.hijri.date || '—'); put('event',s.hijri.event || 'None today');
    for(const name of ['fajr','sunrise','dhuhr','asr','maghrib','isha']) put(name,s.prayers[name]);
    put('prayerNote',s.prayers.ready?'':'Prayer times appear after time sync and schedule load.');
    const prohibitedWindows=[
      ['afterSunrise','afterSunrise'],
      ['atZawal','atZawal'],
      ['beforeSunset','beforeSunset']
    ];
    for(const [key,id] of prohibitedWindows) {
      const window=s.prohibited?.[key];
      put(`${id}Range`,s.prohibited?.ready && window ? `${window.start} – ${window.end}` : '—');
      const state=byId(`${id}State`);
      state.textContent=s.prohibited?.ready && window
        ? (window.active?'Active now':'Not active now')
        : 'Schedule unavailable';
      state.classList.toggle('prohibited-active',Boolean(s.prohibited?.ready && window?.active));
    }
    put('prohibitedNote',s.prohibited?.ready
      ? 'Calculated from today’s schedule and the prohibited-time offsets in Device settings.'
      : 'Prohibited windows appear after time sync and prayer schedule load.');
    put('pomodoroStatus',`${s.pomodoro.phase} · ${s.pomodoro.remaining} · cycle ${s.pomodoro.cycle}/${s.pomodoro.cycles}`);
    put('dhikrStatus',`Count: ${s.dhikrCount}`);
    if(s.quran) {
      const qNum=s.quran.surahNumber||'';
      const qName=s.quran.surah||'None';
      put('quranStatus',`Surah ${qName} · ${s.quran.status||'Idle'}`);
      put('quranNowPlaying',qNum?`${qNum}. ${qName}`:qName);
        const phoneAudio=byId('phoneQuranAudio');
        const phonePlaying=byId('quranOutput').value==='phone' &&
          phoneAudio && !phoneAudio.paused && Boolean(phoneAudio.currentSrc);
        put('quranStreamStatus',phonePlaying?'Playing on this phone':
          (s.quran.playing?'Playing on Prayer-Bot':(s.quran.status||'Idle')));
      const qStatEl=byId('quranStreamStatus');
      if(qStatEl) qStatEl.style.color=s.quran.playing?'var(--accent)':'var(--muted)';
      const qSel=byId('quranSurahSelect');
      if(qSel && s.quran.surahNumber && document.activeElement!==qSel) {
        qSel.value=s.quran.surahNumber;
      }
    }
    put('weatherCity',s.weather.city || '—');
    put('temperature',s.weather.valid?`${s.weather.temperatureC} °C`:'Unavailable');
    put('humidity',s.weather.valid?`${s.weather.humidity}%`:'—');
    put('aqi',s.weather.valid?String(s.weather.aqi):'—');
    put('pm25',s.weather.valid?`${s.weather.pm25} µg/m³`:'—');
    byId('pomodoroStatus').textContent=s.pomodoro.phase==='Waiting for touch'
      ? `Focus complete · touch the sensor to start the ${s.pomodoro.nextBreakMinutes}-minute break`
      : `${s.pomodoro.phase} · ${s.pomodoro.remaining} · cycle ${s.pomodoro.cycle}/${s.pomodoro.cycles}`;
    if(initialStatus) { byId('screenSelect').value=s.screen; initialStatus=false; }
  }
  function applyLocationPreset() {
    const location=locations[byId('locationPreset').value];
    if(!location) return;
    byId('city').value=location.city; byId('lat').value=location.lat;
    byId('lon').value=location.lon; byId('timezone').value=location.tz;
    byId('utcOffset').value=location.utcOff;
  }
  ['city','lat','lon','timezone','utcOffset'].forEach(id=>{
    byId(id).addEventListener('input',()=>byId('locationPreset').value='custom');
  });
  function drawWeeklyChart(days) {
    const canvas=byId('weeklyChart'); const rect=canvas.getBoundingClientRect();
    if(!rect.width || !rect.height) return;
    const ratio=window.devicePixelRatio||1;
    canvas.width=Math.round(rect.width*ratio); canvas.height=Math.round(rect.height*ratio);
    const ctx=canvas.getContext('2d'); ctx.setTransform(ratio,0,0,ratio,0,0);
    const w=rect.width,h=rect.height,pad={l:38,r:12,t:16,b:32};
    ctx.clearRect(0,0,w,h);
    const maxMinutes=Math.max(60,...days.map(day=>day.focusMinutes));
    const maxSessions=Math.max(1,...days.map(day=>day.started));
    const chartW=w-pad.l-pad.r,chartH=h-pad.t-pad.b,step=chartW/7;
    ctx.font='12px system-ui'; ctx.textAlign='right'; ctx.textBaseline='middle';
    for(let tick=0;tick<=3;tick++) {
      const y=pad.t+chartH*tick/3,minutes=Math.round(maxMinutes*(1-tick/3));
      ctx.strokeStyle='#294238';ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(w-pad.r,y);ctx.stroke();
      ctx.fillStyle='#a6b9ad';ctx.fillText(`${minutes}m`,pad.l-6,y);
    }
    ctx.textAlign='center';ctx.textBaseline='top';
    days.forEach((day,i)=>{
      const x=pad.l+step*(i+.5),barH=chartH*day.focusMinutes/maxMinutes;
      ctx.fillStyle='#7fe0ad';ctx.globalAlpha=.75;
      ctx.fillRect(x-step*.24,pad.t+chartH-barH,step*.48,barH);
      ctx.globalAlpha=1;ctx.fillStyle='#a6b9ad';ctx.fillText(day.weekday,x,h-pad.b+8);
    });
    ctx.strokeStyle='#ffd479';ctx.lineWidth=2;ctx.beginPath();
    days.forEach((day,i)=>{
      const x=pad.l+step*(i+.5);
      const y=pad.t+chartH-chartH*day.started/Math.max(maxSessions,4);
      if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);
    });
    ctx.stroke();
    days.forEach((day,i)=>{
      const x=pad.l+step*(i+.5);
      const y=pad.t+chartH-chartH*day.started/Math.max(maxSessions,4);
      ctx.fillStyle='#ffd479';ctx.beginPath();ctx.arc(x,y,3,0,Math.PI*2);ctx.fill();
    });
  }
  async function loadWeeklyStats() {
    try {
      const stats=await api('/api/stats');
      if(!Array.isArray(stats.week) || stats.week.length!==7) throw new Error('Server returned invalid weekly statistics.');
      put('weeklyStarted',stats.totalStarted);put('weeklyCompleted',stats.totalCompleted);
      put('weeklyFocus',`${stats.totalFocusMinutes} min`);drawWeeklyChart(stats.week);
      put('statsNote','Bars show completed focus minutes; the line shows sessions started each day.');
    } catch(e) { put('statsNote',e.message); }
  }
  async function refreshAll() {
    try {
      await Promise.all([loadSettings(),loadStatus(),syncDeviceClock()]);
    }
    catch(e) { message(e.message,true); }
  }
  async function saveSettings() {
    const body={
      city:byId('city').value.trim(), lat:Number(byId('lat').value), lon:Number(byId('lon').value),
      timezone:byId('timezone').value.trim(), utcOff:Number(byId('utcOffset').value),
      calcMethod:Number(byId('calcMethod').value), asrSchool:Number(byId('asrSchool').value),
      hijriOffset:Number(byId('hijriOffset').value||0), azanOn:byId('azanOn').checked,
      volume:Number(byId('volume').value), focusMin:Number(byId('focusMin').value),
      shortBrk:Number(byId('shortBrk').value), longBrk:Number(byId('longBrk').value),
      pomCycles:Number(byId('pomCycles').value),
      sunriseOff:Number(byId('sunriseOff').value),zawalOff:Number(byId('zawalOff').value),
      sunsetOff:Number(byId('sunsetOff').value)
    };
    const owm=byId('owmKey')?.value.trim();
    if(owm) body.owmKey=owm;
    const gemini=byId('geminiKey')?.value.trim();
    if(gemini) body.geminiKey=gemini;

    const inputs=[...document.querySelectorAll('#city,#timezone,#lat,#lon,#utcOffset,#hijriOffset,#focusMin,#shortBrk,#longBrk,#pomCycles,#sunriseOff,#zawalOff,#sunsetOff')];
    const invalid=inputs.find(input=>!input.checkValidity());
    if(invalid){
      byId('settings').open=true;
      invalid.scrollIntoView({behavior:'smooth',block:'center'});
      invalid.focus();
      invalid.reportValidity();
      message(`Check the ${invalid.labels[0]?.textContent||invalid.id} field.`,true);
      return;
    }
    const buttons=[...document.querySelectorAll('#saveButton, #settings button.primary')];
    const controls=[...byId('settings').querySelectorAll('input,select')];
    const previousDisabled=controls.map(control=>control.disabled);
    controls.forEach(control=>control.disabled=true);
    buttons.forEach(b=>{b.disabled=true;b.textContent='Saving…';});
    let saveCompleted=false;
    try {
      await api('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
      saveCompleted=true;
      if(byId('owmKey')) byId('owmKey').value='';
      if(byId('geminiKey')) byId('geminiKey').value='';
      await loadSettings();
      await loadStatus();
      message('Settings saved to device storage and applied successfully.');
    } catch(e) {
      message(saveCompleted
        ? `Settings were saved, but could not be reloaded for verification: ${e.message}`
        : e.message,true);
    } finally {
      controls.forEach((control,index)=>control.disabled=previousDisabled[index]);
      buttons.forEach(b=>{b.disabled=false;b.textContent='Save & apply settings';});
    }
  }
  async function sendControl(body) {
    try { await api('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}); message('Command sent to device.'); }
    catch(e) { message(e.message,true); }
  }
  const surahs=[
    "Al-Fatiha","Al-Baqarah","Aal-E-Imran","An-Nisa","Al-Maidah",
    "Al-An'am","Al-A'raf","Al-Anfal","At-Tawbah","Yunus",
    "Hud","Yusuf","Ar-Ra'd","Ibrahim","Al-Hijr",
    "An-Nahl","Al-Isra","Al-Kahf","Maryam","Taha",
    "Al-Anbiya","Al-Hajj","Al-Mu'minun","An-Nur","Al-Furqan",
    "Ash-Shu'ara","An-Naml","Al-Qasas","Al-Ankabut","Ar-Rum",
    "Luqman","As-Sajdah","Al-Ahzab","Saba","Fatir",
    "Ya-Sin","As-Saffat","Sad","Az-Zumar","Ghafir",
    "Fussilat","Ash-Shura","Az-Zukhruf","Ad-Dukhan","Al-Jathiyah",
    "Al-Ahqaf","Muhammad","Al-Fath","Al-Hujurat","Qaf",
    "Adh-Dhariyat","At-Tur","An-Najm","Al-Qamar","Ar-Rahman",
    "Al-Waqi'ah","Al-Hadid","Al-Mujadila","Al-Hashr","Al-Mumtahanah",
    "As-Saf","Al-Jumu'ah","Al-Munafiqun","At-Taghabun","At-Talaq",
    "At-Tahrim","Al-Mulk","Al-Qalam","Al-Haqqah","Al-Ma'arij",
    "Nuh","Al-Jinn","Al-Muzzammil","Al-Muddaththir","Al-Qiyamah",
    "Al-Insan","Al-Mursalat","An-Naba","An-Nazi'at","Abasa",
    "At-Takwir","Al-Infitar","Al-Mutaffifin","Al-Inshiqaq","Al-Buruj",
    "At-Tariq","Al-A'la","Al-Ghashiyah","Al-Fajr","Al-Balad",
    "Ash-Shams","Al-Lail","Ad-Duha","Ash-Sharh","At-Tin",
    "Al-Alaq","Al-Qadr","Al-Bayyinah","Az-Zalzalah","Al-Adiyat",
    "Al-Qari'ah","At-Takathur","Al-Asr","Al-Humazah","Al-Fil",
    "Quraysh","Al-Ma'un","Al-Kawthar","Al-Kafirun","An-Nasr",
    "Al-Masad","Al-Ikhlas","Al-Falaq","An-Nas"
  ];
  function initQuranDropdown() {
    const sel=byId('quranSurahSelect');
    if(!sel) return;
    sel.replaceChildren();
    surahs.forEach((name,i)=>{
      sel.add(new Option(`${i+1}. ${name}`,i+1));
    });
    sel.value=1;
  }
  let browserRecognition=null;
  let voiceAttemptGeneration=0;
  function setBrowserVoiceState(listening,text,error=false) {
    byId('voiceOrb').classList.toggle('listening',listening);
    byId('browserVoiceButton').disabled=listening||!routerWifiConnected;
    byId('browserVoiceCancel').hidden=!listening;
    byId('browserVoiceStatus').textContent=text;
    byId('browserVoiceStatus').style.color=error?'var(--bad)':'var(--text)';
  }
  function normalizeSurahSpeech(value) {
    return value.toLowerCase().replace(/[^\p{L}\p{N}]+/gu,' ').trim().replace(/\s+/g,' ');
  }
  function findSpokenSurah(transcript) {
    const spoken=normalizeSurahSpeech(transcript);
    const padded=` ${spoken} `;
    for(let i=0;i<surahs.length;i++) {
      const name=normalizeSurahSpeech(surahs[i]);
      if(padded.includes(` ${name} `)) return i+1;
    }
    const aliases={
      'yasin':36,'ya sin':36,'yasīn':36,'ইয়াসিন':36,'ইয়াসীন':36,'ইয়াসিন':36,'ইয়াসীন':36,
      'rahman':55,'ar rahman':55,'আর রহমান':55,'আর রাহমান':55,'রহমান':55,
      'kahf':18,'কাহফ':18,'কাহাফ':18,
      'fatiha':1,'ফাতিহা':1,'আল ফাতিহা':1,
      'baqarah':2,'বাকারা':2,'বাকারাহ':2,'আল বাকারা':2,
      'mulk':67,'মুলক':67,'আল মুলক':67,
      'waqiah':56,'waqia':56,'ওয়াকিয়া':56,'ওয়াকিয়া':56,'ওয়াকিয়াহ':56,
      'ikhlas':112,'ইখলাস':112,'আল ইখলাস':112,
      'falaq':113,'ফালাক':113,'আল ফালাক':113,
      'nas':114,'নাস':114,'আন নাস':114,
      'yusuf':12,'ইউসুফ':12,'maryam':19,'মারইয়াম':19,'মারইয়াম':19,
      'taha':20,'ত্বহা':20,'ta ha':20,'naba':78,'নাবা':78,
      'muzammil':73,'মুজ্জাম্মিল':73,'muddaththir':74,'মুদ্দাসসির':74
    };
    for(const [name,number] of Object.entries(aliases)) {
      if(padded.includes(` ${normalizeSurahSpeech(name)} `)) return number;
    }
    return 0;
  }
  async function chooseSurah(number) {
    await ensureRouterConnection();
    if(!Number.isInteger(number)||number<1||number>surahs.length) throw new Error('Could not identify a Surah. Try again or select one manually.');
    byId('quranSurahSelect').value=number;
    const name=surahs[number-1];
    await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'select',surah:number})});
    put('quranNowPlaying',`${number}. ${name}`);
    if(byId('quranOutput').value==='phone') {
      await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'stop'})});
      const audio=byId('phoneQuranAudio');
      audio.dataset.fallbackUsed='false';
      audio.onerror=()=>{
        if(audio.dataset.fallbackUsed==='false') {
          audio.dataset.fallbackUsed='true';
          audio.src=`https://cdn.islamic.network/quran/audio/128/ar.alafasy/${number}.mp3`;
          put('quranStreamStatus','Primary audio source failed; trying alternate…');
          audio.play().then(()=>{
            put('quranStreamStatus','Playing on this phone (alternate source)');
          }).catch(()=>{
            put('quranStreamStatus','Tap play on the audio controls to start.');
          });
        } else {
          put('quranStreamStatus','Audio could not load. Check internet and try again.');
        }
      };
      audio.src=`https://cdn.mp3quran.net/audio/maher-muaiqly/r1/${String(number).padStart(3,'0')}.mp3`;
      audio.style.display='block';
      put('quranStreamStatus','Loading on this phone…');
      try {
        await audio.play();
        put('quranStreamStatus','Playing on this phone');
      } catch(e) {
        put('quranStreamStatus',audio.dataset.fallbackUsed==='true'
          ? 'Trying alternate audio source…'
          : 'Tap play on the audio controls to start.');
      }
      return;
    }
    byId('phoneQuranAudio').pause();
    const response=await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'play',surah:number})});
    put('quranStreamStatus',response.status==='queued'?'Connecting to Prayer-Bot speaker…':'Starting recitation…');
    await loadStatus();
  }
  async function recognizeTranscriptWithGemini(transcript) {
    const queued=await api('/api/quran/recognize',{
      method:'POST',
      headers:{'Content-Type':'application/json'},
      body:JSON.stringify({transcript})
    });
    if(!Number.isInteger(queued.requestId)||queued.requestId<1) {
      throw new Error('The device did not return a valid Gemini recognition request.');
    }
    for(let attempt=0;attempt<60;attempt++) {
      await new Promise(resolve=>setTimeout(resolve,750));
      const result=await api(`/api/quran/recognize?id=${encodeURIComponent(queued.requestId)}`);
      if(result.status==='done'&&Number.isInteger(result.surahNumber)) return result.surahNumber;
      if(result.status==='error') throw new Error(result.error||'Gemini could not identify a Surah.');
      if(result.status==='unknown') throw new Error('Gemini recognition request expired. Try again.');
      if(result.status!=='queued'&&result.status!=='processing') {
        throw new Error('The device returned an invalid Gemini recognition status.');
      }
      if(attempt%4===0)
        setBrowserVoiceState(true,'Gemini is identifying the Surah from your speech…');
    }
    throw new Error('Gemini recognition timed out. Check the device internet connection and try again.');
  }
  let processingSpokenResult=false;
  async function listenForSurah() {
    const attempt=++voiceAttemptGeneration;
    setBrowserVoiceState(true,'Checking router connection…');
    try {
      await ensureRouterConnection();
    } catch(e) {
      if(attempt!==voiceAttemptGeneration) return;
      setBrowserVoiceState(false,e.message,true);
      return;
    }
    const Recognition=window.SpeechRecognition||window.webkitSpeechRecognition;
    if(!Recognition) {
      if(attempt!==voiceAttemptGeneration) return;
      setBrowserVoiceState(false,'Voice input is not supported in this browser. Select a Surah manually.',true);
      return;
    }
    if(browserRecognition) browserRecognition.abort();
    browserRecognition=new Recognition();
    browserRecognition.lang=byId('voiceLanguage').value;
    browserRecognition.continuous=false;
    browserRecognition.interimResults=true;
    browserRecognition.onstart=()=>{
      if(attempt===voiceAttemptGeneration)
        setBrowserVoiceState(true,'Listening — say a Surah name…');
    };
    browserRecognition.onresult=async event=>{
      if(attempt!==voiceAttemptGeneration) return;
      const result=event.results[event.results.length-1];
      const transcript=result[0].transcript;
      setBrowserVoiceState(true,`I heard: “${transcript}”`);
      if(!result.isFinal) return;
      processingSpokenResult=true;
      setBrowserVoiceState(true,'Sending transcript securely to Gemini on Prayer-Bot…');
      let number=0;
      let fallbackUsed=false;
      try {
        try {
          number=await recognizeTranscriptWithGemini(transcript);
        } catch(geminiError) {
          if(attempt!==voiceAttemptGeneration) return;
          number=findSpokenSurah(transcript);
          if(!number) throw geminiError;
          fallbackUsed=true;
        }
        if(attempt!==voiceAttemptGeneration) return;
        setBrowserVoiceState(false,
          `${fallbackUsed?'Gemini unavailable; local match':'Gemini recognized'}: Surah ${number}, ${surahs[number-1]}`);
        await chooseSurah(number);
      } catch(e) {
        if(attempt===voiceAttemptGeneration)
          setBrowserVoiceState(false,e.message,true);
      } finally {
        if(attempt===voiceAttemptGeneration) processingSpokenResult=false;
      }
    };
    browserRecognition.onerror=event=>{
      if(attempt!==voiceAttemptGeneration) return;
      processingSpokenResult=false;
      const reason=event.error==='not-allowed'
        ? 'Allow microphone access and try again.'
        : `Voice input failed (${event.error}).`;
      setBrowserVoiceState(false,reason,true);
    };
    browserRecognition.onend=()=>{
      if(attempt===voiceAttemptGeneration&&!processingSpokenResult&&byId('voiceOrb').classList.contains('listening'))
        setBrowserVoiceState(false,'No final Surah name detected. Try again.',true);
    };
    try {
      browserRecognition.start();
    } catch(e) {
      setBrowserVoiceState(false,'Could not start voice recognition. Check microphone permission and try again.',true);
    }
  }
  function cancelBrowserVoice() {
    ++voiceAttemptGeneration;
    processingSpokenResult=false;
    if(browserRecognition) browserRecognition.abort();
    browserRecognition=null;
    setBrowserVoiceState(false,'Voice input cancelled.');
  }
  async function playQuran() {
    const sel=byId('quranSurahSelect');
    const surah=Number(sel?.value)||1;
    try {
      await chooseSurah(surah);
      message(`Selected Surah ${surah}: ${surahs[surah-1]||''}.`);
    } catch(e) { message(e.message,true); }
  }
  async function stopQuran() {
    try {
      const audio=byId('phoneQuranAudio');
      audio.pause();
      audio.dataset.fallbackUsed='false';
      await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'stop'})});
      message('Quran recitation stopped.');
      await loadStatus();
    } catch(e) { message(e.message,true); }
  }
  async function nextQuran() {
    try {
      const next=(Number(byId('quranSurahSelect').value)%surahs.length)+1;
      if(byId('quranOutput').value==='phone') await chooseSurah(next);
      else await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'next'})});
      await loadStatus();
    } catch(e) { message(e.message,true); }
  }
  async function prevQuran() {
    try {
      const current=Number(byId('quranSurahSelect').value)||1;
      const previous=current<=1?surahs.length:current-1;
      if(byId('quranOutput').value==='phone') await chooseSurah(previous);
      else await api('/api/quran',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'prev'})});
      await loadStatus();
    } catch(e) { message(e.message,true); }
  }
  async function saveWifi() {
    const ssid=byId('wifiSsid').value;
    const password=byId('wifiPass').value;
    if(!ssid.trim()) {
      byId('wifiMessage').textContent='Enter a Wi-Fi network name.';
      byId('wifiMessage').classList.add('error');
      return;
    }
    if(password && (password.length<8 || password.length>63)) {
      byId('wifiMessage').textContent='A Wi-Fi password must contain 8 to 63 characters, or be blank for an open network.';
      byId('wifiMessage').classList.add('error');
      byId('wifiPass').focus();
      return;
    }
    byId('connectWifi').disabled=true;
    byId('continueWifi').hidden=true;
    try {
      await api('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,pass:password})});
      wifiSubmitted=true;
      byId('wifiMessage').textContent='Connection attempt accepted. Waiting for the router and DHCP address; credentials are not saved until this succeeds…';
      byId('wifiMessage').classList.remove('error');
    } catch(e) {
      byId('wifiMessage').textContent=e.message;
      byId('wifiMessage').classList.add('error');
      byId('connectWifi').disabled=false;
    }
  }
  async function scanWifiNetworks() {
    const button=byId('scanWifi'), select=byId('wifiNetworks'), message=byId('wifiMessage');
    button.disabled=true;
    message.textContent='Scanning for nearby Wi-Fi networks…';
    message.classList.remove('error');
    select.replaceChildren(new Option('Scanning…',''));
    try {
      await api('/api/wifi/scan',{method:'POST'});
      let result;
      for(let attempt=0;attempt<20;attempt++) {
        await new Promise(resolve=>setTimeout(resolve,1000));
        result=await api('/api/wifi/scan');
        if(result.status!=='scanning') break;
      }
      if(!result || result.status==='scanning') throw new Error('Wi-Fi scan timed out. Try again.');
      if(!Array.isArray(result.networks)) throw new Error('Server returned an invalid Wi-Fi scan result.');
      select.replaceChildren(new Option('Select a network',''));
      const strongestBySsid=new Map();
      for(const network of result.networks) {
        if(typeof network.ssid!=='string' || !network.ssid) continue;
        const previous=strongestBySsid.get(network.ssid);
        if(!previous || Number(network.rssi)>Number(previous.rssi)) strongestBySsid.set(network.ssid,network);
      }
      const networks=[...strongestBySsid.values()].sort((a,b)=>Number(b.rssi)-Number(a.rssi));
      for(const network of networks) {
        const option=new Option(`${network.ssid} · ${network.rssi} dBm${network.secured?' · secured':''}`,network.ssid);
        select.add(option);
      }
      if(select.options.length===1) {
        message.textContent='No Wi-Fi networks found. Check range and try again, or enter the network name below.';
      } else {
        message.textContent=`Found ${select.options.length-1} network(s). Select one or enter a network name below.`;
      }
    } catch(e) {
      select.replaceChildren(new Option('Scan to find networks',''));
      message.textContent=e.message;
      message.classList.add('error');
    } finally {
      button.disabled=false;
    }
  }
  byId('wifiNetworks').addEventListener('change',event=>{
    if(event.target.value) {
      const ssid=byId('wifiSsid');
      if(ssid.value!==event.target.value) byId('wifiPass').value='';
      ssid.value=event.target.value;
      byId('wifiPass').focus();
      byId('wifiMessage').textContent='Network selected. Enter its Wi-Fi password, then connect.';
      byId('wifiMessage').classList.remove('error');
    }
  });
  initQuranDropdown();
  refreshAll();
  loadWeeklyStats();
  setInterval(()=>loadStatus().catch(e=>message(e.message,true)),5000);
  setInterval(()=>loadWeeklyStats(),30000);
  setInterval(()=>{if(wifiSubmitted)loadStatus().catch(()=>{
    byId('wifiMessage').textContent='Prayer-Bot is reconnecting or restarting. Stay connected to the Prayer-Bot hotspot; this page will resume checking shortly.';
    byId('wifiMessage').classList.remove('error');
  });},2000);
  window.addEventListener('resize',()=>loadWeeklyStats());
</script>
</body>
</html>
)rawliteral";

#endif
