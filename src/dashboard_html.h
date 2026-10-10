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
    @media(max-width:760px){main{padding:22px 16px 44px}.grid>.card:not(.wide){grid-column:span 12}.prayers{grid-template-columns:repeat(3,minmax(0,1fr))}}
    @media(max-width:480px){main{padding:16px 12px 36px}.card{padding:16px;border-radius:14px}.fields{grid-template-columns:1fr}.stats{grid-template-columns:repeat(2,minmax(0,1fr))}.chart-wrap{height:220px}.actions button{flex-basis:100%}}
    @media(prefers-reduced-motion:reduce){*,*:before,*:after{scroll-behavior:auto!important;transition:none!important}}
  </style>
</head>
<body>
<main>
  <header>
    <div><h1>Prayer-Bot</h1><div class="badge">Your prayer &amp; focus companion</div></div>
    <div class="actions">
      <button type="button" onclick="openWifiSetup()">Wi-Fi setup</button>
      <button type="button" onclick="refreshAll()">Refresh</button>
    </div>
  </header>

  <section id="wifiSetup" class="card wifi" hidden>
    <h2>Connect Prayer-Bot to Wi-Fi</h2>
    <p>Connect to your home Wi-Fi to enable prayer times, weather, and online clock sync. The setup network stays available while connecting.</p>
    <p class="hint">Setup address: <strong>http://192.168.4.1</strong> · Wi-Fi: <strong>PrayerBot-Setup</strong></p>
    <div class="fields" style="margin-top:16px">
      <div class="field"><label for="wifiSsid">Wi-Fi network name (SSID)</label><input id="wifiSsid" maxlength="32" autocomplete="username" required></div>
      <div class="field"><label for="wifiPass">Wi-Fi password</label><input id="wifiPass" type="password" maxlength="63" autocomplete="new-password"></div>
    </div>
    <div class="actions">
      <button id="connectWifi" class="primary" type="button" onclick="saveWifi()">Connect to Wi-Fi</button>
      <button id="continueWifi" type="button" onclick="continueToDashboard()" hidden>Next: open dashboard</button>
    </div>
    <p id="wifiMessage" class="wifi-status" role="status" aria-live="polite"></p>
  </section>

  <section class="grid" id="dashboardContent" hidden>
    <article class="card wide">
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

    <article class="card wide">
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

    <article class="card wide">
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

    <article class="card">
      <h2>Quick access</h2>
      <div class="field">
        <label for="screenSelect">Show screen on device</label>
        <select id="screenSelect">
          <option value="0">Prayer focus</option><option value="1">All prayer times</option>
          <option value="2">Clock &amp; calendar</option><option value="3">Weather</option>
          <option value="4">Pomodoro</option><option value="5">Zikir counter</option>
        </select>
      </div>
      <div class="actions">
        <button type="button" onclick="sendControl({screen:Number(byId('screenSelect').value)})">Open screen</button>
      </div>
      <h3 style="margin-top:18px">Pomodoro</h3>
      <p id="pomodoroStatus">Loading…</p>
      <div class="actions">
        <button type="button" onclick="sendControl({pomodoro:'toggle'})">Start / pause</button>
        <button type="button" onclick="sendControl({pomodoro:'reset'})">Reset</button>
      </div>
      <h3 style="margin-top:18px">Zikir</h3>
      <p id="dhikrStatus">Count: 0</p>
      <button type="button" onclick="sendControl({dhikr:'count'})">Add zikir count</button>
      <p class="hint">Touch either device pad or tap here to count. The Arabic phrase changes every 20 seconds.</p>
      <h3 style="margin-top:18px">Prayer alarm</h3>
      <button type="button" onclick="sendControl({stopAzan:true})">Stop active alarm</button>
    </article>

    <article class="card">
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

    <details class="card wide settings">
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

      <h3>Weather API</h3>
      <div class="field"><label for="owmKey">OpenWeather API key</label><input id="owmKey" type="password" maxlength="47" autocomplete="new-password" placeholder="Not configured"></div>
      <p class="hint">Key status: <span id="owmKeyStatus">Loading…</span>. Leave blank to keep the saved key; a new value replaces it.</p>
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
    const response=await fetch(path,options);
    const data=await response.json();
    if(!data || typeof data!=='object' || Array.isArray(data)) throw new Error('Server returned an invalid JSON response.');
    if(!response.ok) throw new Error(data.error || `Request failed (${response.status})`);
    return data;
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
    byId('owmKey').placeholder=s.owmKeyConfigured?'Saved — enter a new key to replace':'Not configured';
  }
  async function loadStatus() {
    const s=await api('/api/status');
    put('network',s.network.connected?'Connected to home Wi-Fi':'Setup access point');
    put('ip',s.network.ip);
    byId('ipLink').href=`http://${s.network.ip}/`;
    put('homeIp',s.network.homeIp || 'Not connected');
    if(!dashboardReady && wifiSubmitted) {
      if(s.network.connected && !s.network.connecting) {
        wifiSubmitted=false;
        byId('wifiMessage').textContent='Wi-Fi connected successfully. Select Next to open your dashboard.';
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden=false;
        byId('connectWifi').disabled=false;
      } else if(s.network.connecting) {
        byId('wifiMessage').textContent='Connecting to your Wi-Fi network…';
      } else {
        wifiSubmitted=false;
        byId('wifiMessage').textContent='Could not connect. Check the network name and password, then try again.';
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
    put('pomodoroStatus',`${s.pomodoro.phase} · ${s.pomodoro.remaining} · cycle ${s.pomodoro.cycle}/${s.pomodoro.cycles}`);
    put('dhikrStatus',`Count: ${s.dhikrCount}`);
    put('weatherCity',s.weather.city || '—');
    put('temperature',s.weather.valid?`${s.weather.temperatureC} °C`:'Unavailable');
    put('humidity',s.weather.valid?`${s.weather.humidity}%`:'—');
    put('aqi',s.weather.valid?String(s.weather.aqi):'—');
    put('pm25',s.weather.valid?`${s.weather.pm25} µg/m³`:'—');
    byId('pomodoroStatus').textContent=s.pomodoro.phase==='Waiting for touch'
      ? `Focus complete · touch a sensor to start the ${s.pomodoro.nextBreakMinutes}-minute break`
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
    try { await Promise.all([loadSettings(),loadStatus()]); }
    catch(e) { message(e.message,true); }
  }
  async function saveSettings() {
    const body={
      city:byId('city').value.trim(), lat:Number(byId('lat').value), lon:Number(byId('lon').value),
      timezone:byId('timezone').value.trim(), utcOff:Number(byId('utcOffset').value),
      calcMethod:Number(byId('calcMethod').value), asrSchool:Number(byId('asrSchool').value),
      hijriOffset:Number(byId('hijriOffset').value), azanOn:byId('azanOn').checked,
      volume:Number(byId('volume').value), focusMin:Number(byId('focusMin').value),
      shortBrk:Number(byId('shortBrk').value), longBrk:Number(byId('longBrk').value),
      pomCycles:Number(byId('pomCycles').value),
      sunriseOff:Number(byId('sunriseOff').value),zawalOff:Number(byId('zawalOff').value),
      sunsetOff:Number(byId('sunsetOff').value)
    };
    const inputs=[...document.querySelectorAll('#city,#timezone,#lat,#lon,#utcOffset,#hijriOffset,#focusMin,#shortBrk,#longBrk,#pomCycles,#sunriseOff,#zawalOff,#sunsetOff')];
    const invalid=inputs.find(input=>!input.checkValidity());
    if(invalid){invalid.reportValidity();message(`Check the ${invalid.labels[0]?.textContent||invalid.id} field.`,true);return;}
    const key=byId('owmKey').value.trim(); if(key) body.owmKey=key;
    byId('saveButton').disabled=true;
    try {
      await api('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
      byId('owmKey').value='';
      message('Settings saved. Prayer and weather data will refresh for the selected location.');
      await refreshAll();
    } catch(e) { message(e.message,true); }
    finally { byId('saveButton').disabled=false; }
  }
  async function sendControl(body) {
    try { await api('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}); message('Command sent to device.'); }
    catch(e) { message(e.message,true); }
  }
  async function saveWifi() {
    const ssid=byId('wifiSsid').value;
    if(!ssid.trim()) {
      byId('wifiMessage').textContent='Enter a Wi-Fi network name.';
      byId('wifiMessage').classList.add('error');
      return;
    }
    byId('connectWifi').disabled=true;
    byId('continueWifi').hidden=true;
    try {
      await api('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,pass:byId('wifiPass').value})});
      wifiSubmitted=true;
      byId('wifiMessage').textContent='Credentials saved. Connecting…';
      byId('wifiMessage').classList.remove('error');
      await loadStatus();
    } catch(e) {
      byId('wifiMessage').textContent=e.message;
      byId('wifiMessage').classList.add('error');
      byId('connectWifi').disabled=false;
    }
  }
  refreshAll();
  loadWeeklyStats();
  setInterval(()=>{loadStatus().catch(e=>message(e.message,true));loadWeeklyStats();},30000);
  setInterval(()=>{if(wifiSubmitted)loadStatus().catch(e=>{byId('wifiMessage').textContent=e.message;});},1500);
  window.addEventListener('resize',()=>loadWeeklyStats());
</script>
</body>
</html>
)rawliteral";

#endif
