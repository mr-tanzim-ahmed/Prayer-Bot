#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <meta name="theme-color" content="#065f46">
  <title>Prayer-Bot | Islamic Companion &amp; Focus Dashboard</title>
  <style>
    :root {
      --primary: #0f766e;
      --primary-dark: #064e3b;
      --primary-hover: #047857;
      --primary-soft: #ecfdf5;
      --accent: #d97706;
      --accent-soft: #fef3c7;
      --bg: #f8fafc;
      --panel: #ffffff;
      --panel-subtle: #f1f5f9;
      --line: #e2e8f0;
      --line-hover: #cbd5e1;
      --text: #0f172a;
      --text-muted: #64748b;
      --warn: #d97706;
      --warn-soft: #fffbeb;
      --bad: #dc2626;
      --bad-soft: #fef2f2;
      --good: #16a34a;
      --good-soft: #f0fdf4;
      --radius: 16px;
      --radius-sm: 10px;
      --radius-lg: 22px;
      --shadow-sm: 0 1px 3px rgba(15,23,42,0.06);
      --shadow: 0 4px 14px rgba(15,118,110,0.07);
      --shadow-lg: 0 12px 28px rgba(15,23,42,0.1);
    }
    * { box-sizing: border-box; }
    html { scroll-behavior: smooth; }
    body {
      margin: 0;
      background: var(--bg);
      color: var(--text);
      font: 15px/1.55 Inter, system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
      -webkit-font-smoothing: antialiased;
    }
    a { color: var(--primary); text-decoration: none; }
    a:hover { text-decoration: underline; }
    
    /* Top Sticky App Bar */
    .app-bar {
      position: sticky;
      top: 0;
      z-index: 100;
      background: rgba(255, 255, 255, 0.94);
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--line);
      box-shadow: var(--shadow-sm);
    }
    .app-bar-inner {
      max-width: 1140px;
      margin: auto;
      padding: 12px 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
    }
    .brand {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .brand-icon {
      width: 42px;
      height: 42px;
      border-radius: 12px;
      background: linear-gradient(135deg, var(--primary-dark), var(--primary));
      color: #fff;
      display: grid;
      place-items: center;
      box-shadow: 0 3px 10px rgba(15, 118, 110, 0.3);
    }
    .brand-icon svg { width: 24px; height: 24px; fill: currentColor; }
    .brand-title {
      font-size: 1.35rem;
      font-weight: 750;
      letter-spacing: -0.03em;
      color: var(--primary-dark);
      margin: 0;
      line-height: 1.2;
    }
    .brand-subtitle {
      font-size: 0.78rem;
      color: var(--text-muted);
      font-weight: 500;
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .app-bar-actions {
      display: flex;
      align-items: center;
      gap: 8px;
      flex-wrap: wrap;
    }
    
    /* Horizontal Sticky Navigation Pills */
    .nav-bar {
      max-width: 1140px;
      margin: auto;
      padding: 6px 20px 10px;
      display: flex;
      gap: 8px;
      overflow-x: auto;
      scrollbar-width: none;
      -webkit-overflow-scrolling: touch;
    }
    .nav-bar::-webkit-scrollbar { display: none; }
    .nav-pill {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 8px 14px;
      min-height: 40px;
      border-radius: 20px;
      background: var(--panel-subtle);
      border: 1px solid var(--line);
      color: var(--text);
      font-size: 0.84rem;
      font-weight: 600;
      white-space: nowrap;
      transition: all 0.15s ease;
      cursor: pointer;
    }
    .nav-pill:hover, .nav-pill:focus-visible {
      background: var(--primary-soft);
      border-color: var(--primary);
      color: var(--primary-dark);
      text-decoration: none;
    }
    .nav-pill.active {
      background: var(--primary);
      color: #fff;
      border-color: var(--primary);
    }
    
    /* Layout */
    main {
      max-width: 1140px;
      margin: auto;
      padding: 24px 20px 80px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(12, minmax(0, 1fr));
      gap: 20px;
    }
    .card {
      min-width: 0;
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: var(--radius);
      padding: 22px;
      box-shadow: var(--shadow);
      position: relative;
    }
    .wide { grid-column: span 12; }
    .grid > .card:not(.wide) { grid-column: span 6; }
    [id] { scroll-margin-top: 130px; }
    
    /* Typography & Headers */
    h2 {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 1.15rem;
      font-weight: 700;
      letter-spacing: -0.02em;
      margin: 0 0 16px;
      color: var(--text);
    }
    h2 svg, h2 span.icon {
      color: var(--primary);
      font-size: 1.25rem;
    }
    h3 {
      font-size: 0.95rem;
      font-weight: 650;
      margin: 22px 0 10px;
      color: var(--text);
    }
    p { margin: 6px 0; color: var(--text-muted); }
    .hint { font-size: 0.82rem; color: var(--text-muted); }
    
    /* Status Badges */
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 4px 10px;
      border-radius: 20px;
      font-size: 0.78rem;
      font-weight: 650;
      background: var(--panel-subtle);
      color: var(--text-muted);
      border: 1px solid var(--line);
    }
    .badge-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--text-muted);
    }
    .badge-online { background: var(--good-soft); color: var(--good); border-color: #bbf7d0; }
    .badge-online .badge-dot { background: var(--good); }
    .badge-warn { background: var(--warn-soft); color: var(--warn); border-color: #fde68a; }
    .badge-warn .badge-dot { background: var(--warn); }
    .badge-bad { background: var(--bad-soft); color: var(--bad); border-color: #fecaca; }
    .badge-bad .badge-dot { background: var(--bad); }
    
    /* Hero Next Prayer Banner */
    .hero-banner {
      background: linear-gradient(135deg, #064e3b 0%, #0f766e 65%, #115e59 100%);
      color: #fff;
      border: none;
      padding: 26px 28px;
      box-shadow: 0 10px 30px rgba(6, 78, 59, 0.22);
      border-radius: var(--radius-lg);
      position: relative;
      overflow: hidden;
    }
    .hero-banner::after {
      content: "";
      position: absolute;
      right: -20px;
      bottom: -30px;
      width: 220px;
      height: 220px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(255,255,255,0.09) 0%, rgba(255,255,255,0) 70%);
      pointer-events: none;
    }
    .hero-grid {
      display: grid;
      grid-template-columns: 1.4fr 1fr;
      gap: 24px;
      align-items: center;
      position: relative;
      z-index: 1;
    }
    .hero-label {
      font-size: 0.85rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      color: #a7f3d0;
      font-weight: 700;
      margin-bottom: 6px;
    }
    .hero-prayer-name {
      font-size: clamp(2rem, 5vw, 2.75rem);
      font-weight: 850;
      letter-spacing: -0.04em;
      line-height: 1.1;
      margin: 0;
      color: #ffffff;
      text-shadow: 0 2px 8px rgba(0,0,0,0.15);
    }
    .hero-countdown {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      margin-top: 10px;
      padding: 6px 14px;
      background: rgba(0, 0, 0, 0.2);
      border: 1px solid rgba(255, 255, 255, 0.18);
      border-radius: 20px;
      font-size: 0.95rem;
      font-weight: 650;
      color: #fef08a;
    }
    .hero-stats {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
      background: rgba(0, 0, 0, 0.15);
      border: 1px solid rgba(255, 255, 255, 0.12);
      border-radius: 14px;
      padding: 16px;
    }
    .hero-stat span { display: block; font-size: 0.75rem; color: #a7f3d0; font-weight: 600; text-transform: uppercase; }
    .hero-stat strong { display: block; font-size: 1.05rem; font-weight: 700; color: #fff; margin-top: 2px; }
    
    /* Prayer Cards Grid */
    .prayers-grid {
      display: grid;
      grid-template-columns: repeat(6, minmax(0, 1fr));
      gap: 12px;
      margin-top: 8px;
    }
    .prayer-card {
      background: var(--panel-subtle);
      border: 1.5px solid var(--line);
      border-radius: 14px;
      padding: 16px 10px;
      text-align: center;
      transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative;
    }
    .prayer-card:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 16px rgba(15,118,110,0.1);
      border-color: var(--primary);
    }
    .prayer-card.active-prayer {
      background: var(--primary-soft);
      border-color: var(--primary);
      box-shadow: 0 0 0 3px rgba(15, 118, 110, 0.18);
    }
    .prayer-icon {
      font-size: 1.5rem;
      display: block;
      margin-bottom: 4px;
    }
    .prayer-name {
      font-size: 0.88rem;
      font-weight: 700;
      color: var(--text);
      display: block;
    }
    .prayer-arabic {
      font-size: 0.72rem;
      color: var(--text-muted);
      display: block;
      margin-bottom: 8px;
    }
    .prayer-time {
      font-size: 1.25rem;
      font-weight: 800;
      color: var(--primary-dark);
      letter-spacing: -0.02em;
      font-variant-numeric: tabular-nums;
      display: block;
    }
    .prayer-badge {
      display: inline-block;
      font-size: 0.68rem;
      font-weight: 750;
      text-transform: uppercase;
      padding: 2px 7px;
      border-radius: 10px;
      background: var(--primary);
      color: #fff;
      margin-top: 6px;
    }
    
    /* Prohibited Times Banner & Cards */
    .prohibited-banner {
      display: flex;
      align-items: center;
      gap: 14px;
      padding: 14px 18px;
      border-radius: 12px;
      background: var(--good-soft);
      border: 1px solid #bbf7d0;
      color: #14532d;
      margin-bottom: 16px;
      font-weight: 650;
    }
    .prohibited-banner.active-danger {
      background: var(--bad-soft);
      border-color: #fecaca;
      color: #991b1b;
      animation: pulse-danger 2s infinite ease-in-out;
    }
    @keyframes pulse-danger {
      50% { box-shadow: 0 0 0 4px rgba(220, 38, 38, 0.16); }
    }
    .prohibited-grid {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 14px;
    }
    .prohibited-card {
      background: var(--panel-subtle);
      border: 1.5px solid var(--line);
      border-radius: 14px;
      padding: 16px;
      position: relative;
    }
    .prohibited-card.active-window {
      border-color: var(--bad);
      background: var(--bad-soft);
    }
    .prohibited-title {
      font-size: 0.86rem;
      font-weight: 700;
      color: var(--text);
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .prohibited-range {
      font-size: 1.15rem;
      font-weight: 800;
      color: var(--primary-dark);
      margin: 8px 0 4px;
      display: block;
      font-variant-numeric: tabular-nums;
    }
    .prohibited-state {
      display: inline-block;
      font-size: 0.74rem;
      font-weight: 700;
      color: var(--text-muted);
    }
    .prohibited-active {
      color: var(--bad) !important;
      font-weight: 800 !important;
    }
    
    /* Device Overview Stats */
    .stats {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
      gap: 12px;
    }
    .stat {
      padding: 14px;
      border-radius: 12px;
      background: var(--panel-subtle);
      border: 1px solid var(--line);
    }
    .stat span {
      display: block;
      color: var(--text-muted);
      font-size: 0.76rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.04em;
    }
    .stat strong {
      display: block;
      margin-top: 4px;
      overflow-wrap: anywhere;
      font-size: 1.05rem;
      font-weight: 750;
      color: var(--text);
    }
    
    /* Form Inputs & Touch Targets (48px per modern-web-guidance) */
    .fields {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 14px 16px;
    }
    .field { margin: 0 0 10px; }
    label {
      display: block;
      color: #475569;
      font-size: 0.84rem;
      font-weight: 650;
      margin: 0 0 6px;
    }
    input, select {
      width: 100%;
      min-height: 48px;
      padding: 10px 14px;
      border: 1.5px solid var(--line);
      border-radius: var(--radius-sm);
      background: #ffffff;
      color: var(--text);
      font: inherit;
      font-size: 16px; /* Prevents auto-zoom on iOS */
      transition: border-color 0.15s, box-shadow 0.15s;
    }
    input:focus, select:focus, button:focus-visible, a:focus-visible {
      outline: 3px solid rgba(15, 118, 110, 0.25);
      outline-offset: 2px;
      border-color: var(--primary);
    }
    input[type=checkbox] {
      width: 22px;
      min-height: 22px;
      accent-color: var(--primary);
      cursor: pointer;
    }
    input[type=range] {
      min-height: 38px;
      padding: 0;
      accent-color: var(--primary);
      background: transparent;
      border: none;
    }
    .input-with-button {
      display: flex;
      gap: 8px;
    }
    .input-with-button input { flex: 1; }
    /* Wi‑Fi Section Enhancements */
    #wifiSetup .input-with-button select,
    #wifiSetup .input-with-button input {
      min-height: 56px;
    }
    #wifiSetup .input-with-button button {
      min-width: 96px;
    }

    .toggle {
      display: flex;
      align-items: center;
      gap: 12px;
      min-height: 48px;
      margin: 4px 0 14px;
    }
    .toggle label { margin: 0; color: var(--text); cursor: pointer; font-size: 0.95rem; }
    
    /* Buttons (Min 48px touch target) */
    button {
      min-height: 48px;
      border: 1.5px solid var(--line);
      border-radius: var(--radius-sm);
      background: #ffffff;
      color: var(--text);
      padding: 10px 18px;
      font: inherit;
      font-size: 0.92rem;
      font-weight: 650;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      transition: background 0.15s, border-color 0.15s, transform 0.1s, box-shadow 0.15s;
    }
    button:hover {
      background: var(--panel-subtle);
      border-color: var(--line-hover);
    }
    button:active { transform: translateY(1px); }
    button.primary {
      background: var(--primary);
      color: #ffffff;
      border-color: var(--primary);
      box-shadow: 0 2px 8px rgba(15, 118, 110, 0.25);
    }
    button.primary:hover {
      background: var(--primary-hover);
      border-color: var(--primary-hover);
    }
    button.accent {
      background: var(--accent);
      color: #fff;
      border-color: var(--accent);
    }
    button.accent:hover {
      background: #b45309;
    }
    button:disabled { opacity: 0.55; cursor: wait; }
    .actions {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
    }
    .actions button { flex: 1; }
    
    /* Wi-Fi Card Experience */
    .wifi-step-badge {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      width: 26px;
      height: 26px;
      border-radius: 50%;
      background: var(--primary);
      color: #fff;
      font-size: 0.8rem;
      font-weight: 750;
      margin-right: 6px;
    }
    .wifi-status {
      min-height: 32px;
      margin: 14px 0;
      padding: 10px 14px;
      border-radius: 10px;
      background: var(--primary-soft);
      border: 1px solid #bbf7d0;
      color: var(--primary-dark);
      font-weight: 650;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .wifi-status.error {
      background: var(--bad-soft);
      border-color: #fecaca;
      color: var(--bad);
    }
    
    /* Al-Quran Voice Panel & Visualizer */
    .voice-panel {
      display: grid;
      grid-template-columns: auto 1fr;
      align-items: center;
      gap: 16px;
      padding: 18px;
      background: linear-gradient(135deg, #f0fdf4, #ecfdf5);
      border: 1px solid #bbf7d0;
      border-radius: 16px;
    }
    .voice-orb {
      display: grid;
      place-items: center;
      width: 68px;
      height: 68px;
      border: 2px solid #a7f3d0;
      border-radius: 50%;
      background: #ffffff;
      color: var(--primary);
      transition: transform 0.2s, background 0.2s, box-shadow 0.2s;
    }
    .voice-orb svg { width: 32px; height: 32px; }
    .voice-orb.listening {
      background: #dcfce7;
      border-color: var(--primary);
      animation: voice-pulse 1s ease-in-out infinite;
    }
    .voice-bars {
      display: flex;
      align-items: center;
      gap: 4px;
      height: 32px;
      margin-bottom: 6px;
    }
    .voice-bars i {
      width: 4px;
      height: 8px;
      border-radius: 4px;
      background: var(--primary);
      opacity: 0.35;
      transition: height 0.15s ease;
    }
    .listening .voice-bars i {
      animation: voice-wave 0.75s ease-in-out infinite alternate;
      opacity: 1;
    }
    .voice-bars i:nth-child(2n) { animation-delay: 0.15s; }
    .voice-bars i:nth-child(3n) { animation-delay: 0.3s; }
    @keyframes voice-pulse {
      50% { box-shadow: 0 0 0 10px rgba(15, 118, 110, 0.18); transform: scale(1.05); }
    }
    @keyframes voice-wave { to { height: 28px; } }
    .voice-status { min-height: 24px; font-weight: 650; margin: 4px 0; color: var(--text); }
    
    /* Pomodoro & Dhikr Counters */
    .dhikr-button {
      width: 100%;
      min-height: 64px;
      font-size: 1.15rem;
      font-weight: 800;
      border-radius: 16px;
      background: linear-gradient(135deg, var(--primary), var(--primary-dark));
      color: #fff;
      border: none;
      box-shadow: 0 4px 14px rgba(15, 118, 110, 0.28);
    }
    .dhikr-button:active { transform: scale(0.98); }
    .chart-wrap {
      position: relative;
      width: 100%;
      height: 240px;
      margin-top: 10px;
    }
    canvas { width: 100%; height: 100%; display: block; }
    .legend {
      display: flex;
      gap: 16px;
      flex-wrap: wrap;
      font-size: 0.82rem;
      color: var(--text-muted);
      margin-top: 12px;
    }
    .legend span::before {
      content: "";
      display: inline-block;
      width: 10px;
      height: 10px;
      border-radius: 3px;
      margin-right: 6px;
      background: #70b58b;
    }
    .legend .sessions::before {
      border-radius: 50%;
      background: var(--warn);
    }
    
    /* Settings Accordion */
    .settings {
      padding: 0;
      overflow: hidden;
      border: 1.5px solid var(--line);
    }
    .settings > summary {
      cursor: pointer;
      list-style: none;
      padding: 20px 24px;
      font-weight: 750;
      font-size: 1.1rem;
      display: flex;
      justify-content: space-between;
      align-items: center;
      user-select: none;
    }
    .settings > summary::-webkit-details-marker { display: none; }
    .settings > summary::after {
      content: "+";
      color: var(--primary);
      font-size: 1.5rem;
      font-weight: 400;
      line-height: 1;
    }
    .settings[open] > summary::after { content: "−"; }
    .settings-body {
      padding: 0 24px 24px;
      border-top: 1px solid var(--line);
      background: #fdfefe;
    }
    
    /* Floating Save Bar */
    .savebar {
      position: sticky;
      bottom: 16px;
      z-index: 90;
      margin: 24px auto 0;
      max-width: 560px;
      padding: 12px 16px;
      background: rgba(255, 255, 255, 0.94);
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      border: 1.5px solid var(--line);
      border-radius: 18px;
      box-shadow: var(--shadow-lg);
    }
    .savebar button { width: 100%; min-height: 48px; }
    .message {
      min-height: 24px;
      margin-top: 8px;
      color: var(--primary-dark);
      font-weight: 650;
      text-align: center;
    }
    .message.error { color: var(--bad); }
    
    /* Notice Banners */
    .online-notice {
      margin: 0 0 18px;
      padding: 14px 18px;
      border: 1px solid #fed7aa;
      border-radius: 12px;
      background: #fff7ed;
      color: #9a3412;
      font-weight: 650;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .online-notice[hidden], .wifi[hidden], .grid[hidden], .savebar[hidden], #continueWifi[hidden] {
      display: none !important;
    }
    
    /* Legacy module-menu compatibility container */
    .module-menu { display: none; }

    /* Responsive Queries */
    @media (max-width: 960px) {
      .grid > .card:not(.wide) { grid-column: span 12; }
      .prayers-grid { grid-template-columns: repeat(3, minmax(0, 1fr)); }
      .prohibited-grid { grid-template-columns: 1fr; }
      .hero-grid { grid-template-columns: 1fr; gap: 16px; }
    }
    @media (max-width: 580px) {
      main { padding: 16px 12px 60px; }
      .card { padding: 18px 14px; border-radius: 14px; }
      .fields { grid-template-columns: 1fr; }
      .prayers-grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
      .stats { grid-template-columns: repeat(2, minmax(0, 1fr)); }
      .actions { gap: 8px; }
      .actions button { flex: 1 1 100%; min-height: 48px; }
      .hero-banner { padding: 20px 16px; }
      .chart-wrap { height: 200px; }
    }
    @media (prefers-reduced-motion: reduce) {
      *, *::before, *::after {
        animation: none !important;
        transition: none !important;
        scroll-behavior: auto !important;
      }
    }
  </style>
</head>
<body>

<!-- Top Sticky App Bar -->
<header class="app-bar">
  <div class="app-bar-inner">
    <div class="brand">
      <div class="brand-icon" aria-hidden="true">
        <svg viewBox="0 0 24 24"><path d="M12 2c-.5 0-1 .4-1 .9v1.3C8.5 4.8 6.5 7.1 6.5 10v3l-2 2v1h15v-1l-2-2v-3c0-2.9-2-5.2-4.5-5.8V2.9c0-.5-.5-.9-1-.9zm-3 16h6c0 1.7-1.3 3-3 3s-3-1.3-3-3z"/></svg>
      </div>
      <div>
        <h1 class="brand-title">Prayer-Bot</h1>
        <div class="brand-subtitle">
          <span>Islamic Companion &amp; Focus</span>
          <span id="headerStatusBadge" class="badge badge-online"><span class="badge-dot"></span><span id="networkShort">Checking…</span></span>
        </div>
      </div>
    </div>
    <div class="app-bar-actions">
      <button type="button" onclick="openWifiSetup()" aria-label="Open Wi-Fi setup">📶 Wi-Fi</button>
      <button type="button" class="primary" onclick="refreshAll()" aria-label="Refresh dashboard data">🔄 Refresh</button>
    </div>
  </div>

  <!-- Horizontal Scrollable Navigation Pills -->
  <nav class="nav-bar" aria-label="Quick jump tabs">
    <a class="nav-pill active" href="#overview">🕌 Overview</a>
    <a class="nav-pill" href="#prayer-times">⏰ Prayer Times</a>
    <a class="nav-pill" href="#prohibited-times">🚫 Prohibited Times</a>
    <a class="nav-pill" href="#quran-player">📖 Al-Quran Player</a>
    <a class="nav-pill" href="#pomodoro-stats">⏱ Focus &amp; History</a>
    <a class="nav-pill" href="#dhikr-controls">📿 Dhikr Counter</a>
    <a class="nav-pill" href="#weather">⛅ Weather</a>
    <a class="nav-pill" href="#wifiSetup" onclick="openWifiSetup();return false">📶 Wi-Fi Setup</a>
    <a class="nav-pill" href="#settings" onclick="byId('settings').open=true">⚙ Settings</a>
  </nav>
</header>

<main>
  <!-- Hidden legacy details for selector compatibility -->
  <details class="module-menu" id="modulesMenu" hidden><nav><a href="#overview">Overview</a><a href="#settings">Settings</a></nav></details>

  <!-- Router Disconnected Notice -->
  <aside id="onlineOfflineNotice" class="online-notice" role="status" aria-live="polite" hidden>
    <span style="font-size:1.3rem">⚠️</span>
    <div>Router Wi-Fi is not connected. Internet features (online prayer sync, weather, and Quran audio) are unavailable. Connect to your 2.4 GHz home Wi-Fi below; offline Azan beep, Pomodoro, and Dhikr remain active.</div>
  </aside>

  <!-- Wi-Fi Setup Section -->
  <section id="wifiSetup" class="card wide" style="margin-bottom:24px" hidden>
    <h2><span>📶</span> Wi-Fi Connection Setup</h2>
    <p>Connect Prayer-Bot to your home router. The device verifies the router connection, obtains a DHCP IP, and securely saves credentials.</p>
    <div class="hint" style="margin-bottom:18px;line-height:1.6">
      <strong>Requirements:</strong> 2.4 GHz Wi-Fi only (5 GHz-only SSIDs cannot be used).<br>
      Default setup hotspot: <strong id="setupApNameLabel">Prayer-Bot</strong> (password: <strong>12345678</strong>) · Setup IP: <strong>http://192.168.4.1</strong>
    </div>

    <div class="fields">
      <div class="field">
        <label for="wifiNetworks"><span class="wifi-step-badge">1</span> Nearby Wi-Fi networks</label>
        <div class="input-with-button">
          <select id="wifiNetworks"><option value="">— tap Scan to discover networks —</option></select>
          <button id="scanWifi" type="button" onclick="scanWifiNetworks()" style="flex:0 0 auto">🔍 Scan</button>
        </div>
      </div>
      <div class="field">
        <label for="wifiSsid"><span class="wifi-step-badge">2</span> Wi-Fi network name (SSID)</label>
        <input id="wifiSsid" maxlength="32" autocomplete="username" placeholder="e.g. Firefly" required>
      </div>
      <div class="field" style="grid-column:span 2">
        <label for="wifiPass"><span class="wifi-step-badge">3</span> Wi-Fi password</label>
        <div class="input-with-button">
          <input id="wifiPass" type="password" maxlength="63" autocomplete="current-password" placeholder="Enter Wi-Fi password (blank if open)">
          <button type="button" onclick="togglePasswordVisibility('wifiPass', this)" style="flex:0 0 auto" aria-label="Toggle password view">👁 Show</button>
        </div>
        <small class="hint">WPA2/WPA3 password. For open router networks, leave this blank.</small>
      </div>
    </div>

    <div class="actions" style="margin-top:16px">
      <button id="connectWifi" class="primary" type="button" onclick="saveWifi()">🚀 Connect to Wi-Fi</button>
      <button id="continueWifi" class="accent" type="button" onclick="continueToDashboard()" hidden>Next: Open Dashboard ➔</button>
    </div>
    <div id="wifiMessage" class="wifi-status" role="status" aria-live="polite">Ready to connect.</div>
  </section>

  <!-- Main Dashboard Content -->
  <div class="grid" id="dashboardContent" hidden>
    
    <!-- Hero Banner: Next Prayer & Islamic Calendar -->
    <article class="card wide hero-banner" id="overview">
      <div class="hero-grid">
        <div>
          <div class="hero-label">UPCOMING PRAYER</div>
          <h2 class="hero-prayer-name" id="nextPrayerNameDisplay">Loading…</h2>
          <div class="hero-countdown" id="heroCountdownBadge">
            <span>⏳</span> <span id="nextPrayer">Schedule loading…</span>
          </div>
        </div>
        <div class="hero-stats">
          <div class="hero-stat"><span>Hijri Date</span><strong id="hijri">—</strong></div>
          <div class="hero-stat"><span>Islamic Event</span><strong id="event">—</strong></div>
          <div class="hero-stat"><span>Alarm Status</span><strong id="azanStatus">—</strong></div>
          <div class="hero-stat"><span>Home IP</span><strong id="homeIp">—</strong></div>
        </div>
      </div>
    </article>

    <!-- Quick Device Stats Overview -->
    <article class="card wide">
      <h2><span>📊</span> Device &amp; Connection Status</h2>
      <div class="stats">
        <div class="stat"><span>Connection</span><strong id="network">Loading…</strong></div>
        <div class="stat"><span>Dashboard IP</span><strong><a id="ipLink" href="#"><span id="ip">—</span></a></strong></div>
        <div class="stat"><span>Prayer Sync API</span><strong id="prayerApiStatus">Aladhan Live</strong></div>
        <div class="stat"><span>Weather Location</span><strong id="overviewCity">Dhaka</strong></div>
      </div>
    </article>

    <!-- Prayer Times Today Card -->
    <article class="card wide" id="prayer-times">
      <div style="display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;margin-bottom:12px">
        <h2><span>🕌</span> Today's Prayer Schedule</h2>
        <div class="actions" style="flex:0 0 auto">
          <button type="button" onclick="triggerPrayerSync()" id="syncPrayerBtn" style="min-height:38px;padding:6px 14px;font-size:0.84rem">
            <span>🔄</span> Sync with Aladhan API
          </button>
        </div>
      </div>
      <p class="hint">Calculated using astronomical solar coordinates from Aladhan API with high precision.</p>

      <div class="prayers-grid">
        <div class="prayer-card" id="card-fajr">
          <span class="prayer-icon">🌌</span>
          <span class="prayer-name">Fajr</span>
          <span class="prayer-arabic">الفجر</span>
          <span class="prayer-time" id="fajr">04:41</span>
        </div>
        <div class="prayer-card" id="card-sunrise">
          <span class="prayer-icon">🌅</span>
          <span class="prayer-name">Sunrise</span>
          <span class="prayer-arabic">الشروق</span>
          <span class="prayer-time" id="sunrise">05:57</span>
        </div>
        <div class="prayer-card" id="card-dhuhr">
          <span class="prayer-icon">☀️</span>
          <span class="prayer-name">Dhuhr</span>
          <span class="prayer-arabic">الظهر</span>
          <span class="prayer-time" id="dhuhr">11:44</span>
        </div>
        <div class="prayer-card" id="card-asr">
          <span class="prayer-icon">🌤</span>
          <span class="prayer-name">Asr</span>
          <span class="prayer-arabic">العصر</span>
          <span class="prayer-time" id="asr">15:53</span>
        </div>
        <div class="prayer-card" id="card-maghrib">
          <span class="prayer-icon">🌇</span>
          <span class="prayer-name">Maghrib</span>
          <span class="prayer-arabic">المغرب</span>
          <span class="prayer-time" id="maghrib">17:30</span>
        </div>
        <div class="prayer-card" id="card-isha">
          <span class="prayer-icon">🌙</span>
          <span class="prayer-name">Isha</span>
          <span class="prayer-arabic">العشاء</span>
          <span class="prayer-time" id="isha">18:41</span>
        </div>
      </div>
      <p id="prayerNote" class="hint" style="margin-top:14px">Times automatically update based on your location and calculation method.</p>
    </article>

    <!-- Prohibited Times Card (Makruh Tahrimi) -->
    <article class="card wide" id="prohibited-times">
      <h2><span>🚫</span> Prohibited Prayer Times (Salat Makruh)</h2>
      <div id="prohibitedStatusBanner" class="prohibited-banner">
        <span style="font-size:1.3rem">✅</span>
        <span id="prohibitedBannerText">Permitted Window: Safe to perform voluntary prayers (Nafl).</span>
      </div>

      <div class="prohibited-grid">
        <div class="prohibited-card" id="card-prohibited-sunrise">
          <div class="prohibited-title"><span>🌅</span> 1. After Sunrise (طلوع الشمس)</div>
          <span class="prohibited-range" id="afterSunriseRange">05:57 – 06:12</span>
          <span class="prohibited-state" id="afterSunriseState">Not active now</span>
          <p class="hint">From sunrise until the sun rises a spear's height (~15 min).</p>
        </div>
        <div class="prohibited-card" id="card-prohibited-zawal">
          <div class="prohibited-title"><span>☀️</span> 2. At Zawal (استواء الشمس)</div>
          <span class="prohibited-range" id="atZawalRange">11:34 – 11:44</span>
          <span class="prohibited-state" id="atZawalState">Not active now</span>
          <p class="hint">Exact solar noon just before Dhuhr azan starts (~10 min).</p>
        </div>
        <div class="prohibited-card" id="card-prohibited-sunset">
          <div class="prohibited-title"><span>🌇</span> 3. Before Sunset (غروب الشمس)</div>
          <span class="prohibited-range" id="beforeSunsetRange">17:15 – 17:30</span>
          <span class="prohibited-state" id="beforeSunsetState">Not active now</span>
          <p class="hint">When the sun pales until it sets into Maghrib (~15 min).</p>
        </div>
      </div>
      <p id="prohibitedNote" class="hint" style="margin-top:14px">Calculated directly from today's Sunrise, Dhuhr, and Maghrib times using the configured offsets.</p>
    </article>

    <!-- Al-Quran Audio Player -->
    <article class="card" id="quran-player">
      <h2><span>📖</span> Al-Quran Recitation Player</h2>
      <p class="hint">Recitation by Sheikh Maher Al-Muaiqly. Choose playback on Prayer-Bot speaker or this phone. Use voice recognition to name any Surah in English or Bangla.</p>

      <div class="voice-panel">
        <div id="voiceOrb" class="voice-orb" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="2" width="6" height="12" rx="3"></rect><path d="M5 10v2a7 7 0 0 0 14 0v-2M12 19v3m-4 0h8"></path></svg>
        </div>
        <div>
          <div class="voice-bars" aria-hidden="true"><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i></div>
          <div id="browserVoiceStatus" class="voice-status">Ready to recognize Surah voice command.</div>
          <div class="actions" style="margin-top:8px">
            <select id="voiceLanguage" data-requires-wifi style="max-width:140px;min-height:44px"><option value="en-US">English</option><option value="bn-BD">বাংলা (Bangla)</option></select>
            <button id="browserVoiceButton" type="button" data-requires-wifi onclick="listenForSurah()" style="min-height:44px">🎙 Speak Surah</button>
            <button id="browserVoiceCancel" type="button" onclick="cancelBrowserVoice()" style="min-height:44px" hidden>Cancel</button>
          </div>
        </div>
      </div>

      <div class="field" style="margin-top:16px">
        <label for="quranOutput">Audio Playback Destination</label>
        <select id="quranOutput" data-requires-wifi>
          <option value="device">🔊 Prayer-Bot Device Speaker</option>
          <option value="phone">📱 This Phone / Browser</option>
        </select>
      </div>

      <div class="field">
        <label for="quranSurahSelect">Select Surah (1 to 114)</label>
        <select id="quranSurahSelect" data-requires-wifi>
          <option value="1">1. Al-Fatiha</option>
        </select>
      </div>

      <div class="actions" style="margin-top:12px">
        <button type="button" class="primary" data-requires-wifi onclick="playQuran()">▶ Play Recitation</button>
        <button type="button" onclick="stopQuran()">⏹ Stop</button>
        <button type="button" data-requires-wifi onclick="prevQuran()">⏮ Prev</button>
        <button type="button" data-requires-wifi onclick="nextQuran()">⏭ Next</button>
      </div>

      <audio id="phoneQuranAudio" controls preload="none" data-requires-wifi style="display:none;width:100%;margin-top:14px"></audio>

      <div class="stats" style="margin-top:16px">
        <div class="stat"><span>Now Playing</span><strong id="quranNowPlaying">None</strong></div>
        <div class="stat"><span>Stream Status</span><strong id="quranStreamStatus">Idle</strong></div>
      </div>
      <p id="quranStatus" class="hint" style="margin-top:8px">Surah player status: Loading…</p>
    </article>

    <!-- Quick Device Controls & Screen Selector -->
    <article class="card" id="device-controls">
      <h2><span>🎛</span> Quick Device Controls</h2>
      <div class="field">
        <label for="screenSelect">Switch OLED Screen Display</label>
        <div class="input-with-button">
          <select id="screenSelect">
            <option value="0">1. Prayer Focus (Next prayer + countdown)</option>
            <option value="1">2. All Prayer Times</option>
            <option value="2">3. Clock &amp; Hijri Calendar</option>
            <option value="3">4. Weather &amp; Air Quality</option>
            <option value="4">5. Pomodoro Focus Timer</option>
            <option value="5">6. Dhikr / Zikir Counter</option>
            <option value="6">7. Al-Quran Surah Recitation</option>
          </select>
          <button type="button" onclick="sendControl({screen:Number(byId('screenSelect').value)})">Switch</button>
        </div>
      </div>

      <h3 id="dhikr-controls">📿 Digital Dhikr / Tasbih Counter</h3>
      <p id="dhikrStatus" style="font-size:1.2rem;font-weight:750;color:var(--primary-dark)">Count: 0</p>
      <button class="dhikr-button" type="button" onclick="sendControl({dhikr:'count'})">
        👆 Tap for Dhikr (+1)
      </button>
      <p class="hint">Touch the capacitive sensor on top of Prayer-Bot or tap above. Meaning shifts automatically between SubhanAllah, Alhamdulillah, Allahu Akbar.</p>

      <h3>🔔 Prayer Azan Alarm</h3>
      <div class="actions">
        <button type="button" onclick="sendControl({stopAzan:true})">🔕 Stop Active Beep Alarm</button>
      </div>
    </article>

    <!-- Pomodoro Focus Timer & Weekly Stats -->
    <article class="card wide" id="pomodoro-stats">
      <h2><span>⏱</span> Focus &amp; Pomodoro History</h2>
      <div class="stats" style="margin-bottom:16px">
        <div class="stat"><span>Current Phase</span><strong id="pomodoroStatus">Idle · 25:00</strong></div>
        <div class="stat"><span>Sessions Started (7d)</span><strong id="weeklyStarted">0</strong></div>
        <div class="stat"><span>Sessions Completed</span><strong id="weeklyCompleted">0</strong></div>
        <div class="stat"><span>Total Focus Minutes</span><strong id="weeklyFocus">0 min</strong></div>
      </div>

      <div class="actions" style="margin-bottom:18px">
        <button type="button" class="primary" onclick="sendControl({pomodoro:'toggle'})">⏯ Start / Pause Focus</button>
        <button type="button" onclick="sendControl({pomodoro:'reset'})">↺ Reset Timer</button>
      </div>

      <div class="legend"><span>Completed focus minutes</span><span class="sessions">Sessions started</span></div>
      <div class="chart-wrap"><canvas id="weeklyChart" aria-label="Daily focus chart for the past week"></canvas></div>
      <p id="statsNote" class="hint">Stored in LittleFS flash memory. On the device, long-touch the sensor for 3 seconds to start.</p>
    </article>

    <!-- Weather & Air Quality Card -->
    <article class="card wide" id="weather">
      <h2><span>⛅</span> Local Weather &amp; Air Quality</h2>
      <div class="stats">
        <div class="stat"><span>City</span><strong id="weatherCity">—</strong></div>
        <div class="stat"><span>Temperature</span><strong id="temperature">—</strong></div>
        <div class="stat"><span>Relative Humidity</span><strong id="humidity">—</strong></div>
        <div class="stat"><span>European AQI</span><strong id="aqi">—</strong></div>
        <div class="stat"><span>PM2.5 Concentration</span><strong id="pm25">—</strong></div>
      </div>
      <p class="hint" style="margin-top:12px">Real-time weather data from OpenWeatherMap &amp; Open-Meteo European Air Quality APIs.</p>
    </article>

    <!-- Device Settings Accordion -->
    <details class="card wide settings" id="settings">
      <summary>
        <span><span>⚙</span> Device Preferences &amp; Coordinates</span>
        <span class="badge">Location, Calculation &amp; API Keys</span>
      </summary>
      <div class="settings-body">
        
        <h3 style="margin-top:16px">📍 Location &amp; Coordinates</h3>
        <div class="fields">
          <div class="field" style="grid-column:span 2">
            <label for="locationPreset">Choose Preset Location</label>
            <select id="locationPreset" onchange="applyLocationPreset()">
              <option value="dhaka">Dhaka, Bangladesh (default)</option>
              <option value="chattogram">Chattogram, Bangladesh</option>
              <option value="sylhet">Sylhet, Bangladesh</option>
              <option value="rajshahi">Rajshahi, Bangladesh</option>
              <option value="khulna">Khulna, Bangladesh</option>
              <option value="custom">Custom Location / Coordinates</option>
            </select>
          </div>
          <div class="field"><label for="city">City Name</label><input id="city" maxlength="63" required></div>
          <div class="field"><label for="timezone">Timezone Label</label><input id="timezone" maxlength="39" placeholder="Asia/Dhaka" required></div>
          <div class="field"><label for="lat">Latitude (-90 to 90)</label><input id="lat" type="number" min="-90" max="90" step="any" required></div>
          <div class="field"><label for="lon">Longitude (-180 to 180)</label><input id="lon" type="number" min="-180" max="180" step="any" required></div>
          <div class="field"><label for="utcOffset">UTC Offset (hours, e.g. +6)</label><input id="utcOffset" type="number" min="-12" max="14" step="0.25" required></div>
          <div class="field"><label for="hijriOffset">Hijri Day Offset (-2 to +2)</label><input id="hijriOffset" type="number" min="-2" max="2" step="1"></div>
        </div>

        <h3>🕌 Prayer Calculation Parameters</h3>
        <div class="fields">
          <div class="field">
            <label for="calcMethod">Calculation Method</label>
            <select id="calcMethod">
              <option value="1">1. University of Islamic Sciences, Karachi</option>
              <option value="2">2. Islamic Society of North America (ISNA)</option>
              <option value="3">3. Muslim World League (MWL)</option>
              <option value="4">4. Umm Al-Qura University, Makkah</option>
              <option value="5">5. Egyptian General Authority of Survey</option>
              <option value="0">0. Shia Ithna-Ashari</option>
              <option value="7">7. Institute of Geophysics, Tehran</option>
              <option value="8">8. Gulf Region</option>
              <option value="9">9. Kuwait</option>
              <option value="10">10. Qatar</option>
              <option value="11">11. Majlis Ugama Islam Singapura</option>
              <option value="12">12. Union des Organisations Islamiques de France</option>
              <option value="13">13. Diyanet İşleri Başkanlığı, Turkey</option>
              <option value="14">14. Spiritual Administration of Muslims of Russia</option>
              <option value="15">15. Moonsighting Committee Worldwide</option>
              <option value="16">16. Dubai, UAE</option>
              <option value="17">17. JAKIM, Malaysia</option>
              <option value="20">20. KEMENAG, Indonesia</option>
            </select>
          </div>
          <div class="field">
            <label for="asrSchool">Asr Juristic Method</label>
            <select id="asrSchool">
              <option value="1">Hanafi (Later Asr - Standard for BD/PK/IN/TR)</option>
              <option value="0">Shafi'i / Maliki / Hanbali (Standard)</option>
            </select>
          </div>
        </div>

        <h3>🔔 Azan Beep Alarm</h3>
        <div class="toggle">
          <input id="azanOn" type="checkbox">
          <label for="azanOn">Enable audio beep alarm when prayer time arrives</label>
        </div>
        <div class="field">
          <label for="volume">Alarm Volume: <span id="volumeValue">80</span>%</label>
          <input id="volume" type="range" min="0" max="100" value="80" oninput="byId('volumeValue').textContent=this.value">
        </div>

        <h3>⏱ Pomodoro &amp; Focus Intervals</h3>
        <div class="fields">
          <div class="field"><label for="focusMin">Focus Duration (minutes)</label><input id="focusMin" type="number" min="1" max="120" step="1" required></div>
          <div class="field"><label for="shortBrk">Short Break (minutes)</label><input id="shortBrk" type="number" min="2" max="15" step="1" required></div>
          <div class="field"><label for="longBrk">Long Break (minutes)</label><input id="longBrk" type="number" min="1" max="60" required></div>
          <div class="field"><label for="pomCycles">Sessions Before Long Break</label><input id="pomCycles" type="number" min="1" max="10" required></div>
        </div>

        <h3>🚫 Prohibited-Time Window Offsets</h3>
        <div class="fields">
          <div class="field"><label for="sunriseOff">After Sunrise Window (0–60 min)</label><input id="sunriseOff" type="number" min="0" max="60" required></div>
          <div class="field"><label for="zawalOff">Zawal (Pre-Dhuhr) Window (0–60 min)</label><input id="zawalOff" type="number" min="0" max="60" required></div>
          <div class="field"><label for="sunsetOff">Pre-Sunset Window (0–60 min)</label><input id="sunsetOff" type="number" min="0" max="60" required></div>
        </div>

        <h3>🔑 External API Keys</h3>
        <div class="fields">
          <div class="field">
            <label for="owmKey">OpenWeatherMap API Key</label>
            <div class="input-with-button">
              <input id="owmKey" type="password" maxlength="63" placeholder="Leave blank to retain current key">
              <button type="button" onclick="togglePasswordVisibility('owmKey', this)">👁</button>
            </div>
            <small class="hint">Status: <span id="owmKeyStatus">Loading…</span></small>
          </div>
          <div class="field">
            <label for="geminiKey">Gemini Flash API Key (Voice Surah Search)</label>
            <div class="input-with-button">
              <input id="geminiKey" type="password" maxlength="63" placeholder="Leave blank to retain current key">
              <button type="button" onclick="togglePasswordVisibility('geminiKey', this)">👁</button>
            </div>
            <small class="hint">Status: <span id="geminiKeyStatus">Loading…</span></small>
          </div>
        </div>

        <div class="actions" style="margin-top:20px">
          <button class="primary" type="button" onclick="saveSettings()">💾 Save &amp; Apply All Settings</button>
        </div>
      </div>
    </details>

  </div>

  <!-- Sticky Bottom Save Bar -->
  <div class="savebar" id="savebar" hidden>
    <button id="saveButton" class="primary" type="button" onclick="saveSettings()">💾 Save &amp; Apply Settings</button>
    <div id="message" class="message" role="status" aria-live="polite"></div>
  </div>
</main>

<script>
  const byId = id => document.getElementById(id);
  const put = (id, value) => { const el = byId(id); if (el) el.textContent = value ?? '—'; };
  let initialStatus = true;
  let dashboardReady = false;
  let wifiSubmitted = false;
  let routerWifiConnected = false;

  const locations = {
    dhaka: { city: 'Dhaka', lat: 23.8103, lon: 90.4125, tz: 'Asia/Dhaka', utcOff: 6 },
    chattogram: { city: 'Chattogram', lat: 22.3569, lon: 91.7832, tz: 'Asia/Dhaka', utcOff: 6 },
    sylhet: { city: 'Sylhet', lat: 24.8949, lon: 91.8687, tz: 'Asia/Dhaka', utcOff: 6 },
    rajshahi: { city: 'Rajshahi', lat: 24.3745, lon: 88.6042, tz: 'Asia/Dhaka', utcOff: 6 },
    khulna: { city: 'Khulna', lat: 22.8456, lon: 89.5403, tz: 'Asia/Dhaka', utcOff: 6 }
  };

  function message(text, error = false) {
    const el = byId('message');
    if (!el) return;
    el.textContent = text;
    el.classList.toggle('error', error);
  }

  function togglePasswordVisibility(inputId, btn) {
    const input = byId(inputId);
    if (!input) return;
    if (input.type === 'password') {
      input.type = 'text';
      btn.textContent = '🔒 Hide';
    } else {
      input.type = 'password';
      btn.textContent = '👁 Show';
    }
  }

  function showDashboard() {
    dashboardReady = true;
    byId('wifiSetup').hidden = true;
    byId('dashboardContent').hidden = false;
    byId('savebar').hidden = false;
  }

  function openWifiSetup() {
    byId('wifiSetup').hidden = false;
    byId('continueWifi').hidden = true;
    byId('wifiMessage').textContent = 'Ready to connect to your 2.4 GHz router.';
    byId('wifiMessage').classList.remove('error');
    if (!dashboardReady) {
      byId('dashboardContent').hidden = true;
      byId('savebar').hidden = true;
    }
    byId('wifiSetup').scrollIntoView({ behavior: 'smooth', block: 'start' });
    byId('wifiSsid').focus();
  }

  function continueToDashboard() {
    showDashboard();
    refreshAll();
  }

  async function api(path, options = {}) {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 10000);
    let response, data;
    try {
      response = await fetch(path, { ...options, signal: controller.signal });
      data = await response.json();
    } catch(e) {
      if (e.name === 'AbortError') throw new Error('Device request timed out. Check connection.');
      if (e instanceof SyntaxError) throw new Error('Device returned invalid JSON.');
      throw new Error('Could not reach device. Check Wi-Fi connection.');
    } finally {
      clearTimeout(timeout);
    }
    if (!data || typeof data !== 'object') throw new Error('Invalid JSON response.');
    if (!response.ok) throw new Error(data.error || `Request failed (${response.status})`);
    return data;
  }

  function setOnlineAvailability(connected) {
    routerWifiConnected = Boolean(connected);
    const notice = byId('onlineOfflineNotice');
    notice.hidden = routerWifiConnected;
    
    const badge = byId('headerStatusBadge');
    const netShort = byId('networkShort');
    if (routerWifiConnected) {
      badge.className = 'badge badge-online';
      netShort.textContent = 'Online';
    } else {
      badge.className = 'badge badge-warn';
      netShort.textContent = 'Hotspot AP';
    }

    if (!routerWifiConnected) {
      setBrowserVoiceState(false, 'Connect Wi-Fi to use speech recognition.', true);
    }
    document.querySelectorAll('[data-requires-wifi]').forEach(control => {
      control.disabled = !routerWifiConnected;
    });
    const audio = byId('phoneQuranAudio');
    audio.controls = routerWifiConnected;
    if (!routerWifiConnected && !audio.paused) {
      audio.pause();
      audio.removeAttribute('src');
      audio.load();
      put('quranStreamStatus', 'Stopped: router Wi-Fi is disconnected.');
    }
  }

  async function ensureRouterConnection() {
    const status = await api('/api/status');
    setOnlineAvailability(status.network?.connected);
    if (!routerWifiConnected) throw new Error('Not connected to router Wi-Fi. Connect Wi-Fi and try again.');
  }

  async function syncDeviceClock() {
    const epoch = Math.floor(Date.now() / 1000);
    try {
      await api('/api/time', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ epoch }) });
    } catch(e) {}
  }

  async function loadSettings() {
    const s = await api('/api/settings');
    byId('city').value = s.city; byId('lat').value = s.lat; byId('lon').value = s.lon;
    byId('timezone').value = s.timezone; byId('utcOffset').value = s.utcOff;
    byId('calcMethod').value = s.calcMethod; byId('asrSchool').value = s.asrSchool;
    byId('hijriOffset').value = s.hijriOffset; byId('azanOn').checked = s.azanOn;
    byId('volume').value = s.volume; put('volumeValue', s.volume);
    byId('focusMin').value = s.focusMin; byId('shortBrk').value = s.shortBrk;
    byId('longBrk').value = s.longBrk; byId('pomCycles').value = s.pomCycles;
    byId('sunriseOff').value = s.sunriseOff; byId('zawalOff').value = s.zawalOff;
    byId('sunsetOff').value = s.sunsetOff;
    const preset = Object.keys(locations).find(key => Math.abs(locations[key].lat - s.lat) < 0.01 && Math.abs(locations[key].lon - s.lon) < 0.01);
    byId('locationPreset').value = preset || 'custom';
    put('overviewCity', s.city || 'Dhaka');
    put('owmKeyStatus', s.owmKeyConfigured ? 'Configured ✅' : 'Not configured');
    put('geminiKeyStatus', s.geminiKeyConfigured ? 'Configured ✅' : 'Not configured');
  }

  async function loadStatus() {
    const s = await api('/api/status');
    setOnlineAvailability(s.network?.connected);
    put('network', s.network.connected ? 'Connected to Router' : 'Device Hotspot Mode');
    put('ip', s.network.ip);
    byId('ipLink').href = `http://${s.network.ip}/`;
    put('homeIp', s.network.homeIp || 'Not connected to home network');

    if (!dashboardReady && wifiSubmitted) {
      if (s.network.connectionFailed) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = 'Could not connect. Check network name and password, then try again.';
        byId('wifiMessage').classList.add('error');
        byId('continueWifi').hidden = true;
        byId('connectWifi').disabled = false;
      } else if (s.network.restarting) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = `Connected! Device is restarting. Assigned IP: ${s.network.homeIp || 'check your router'}.`;
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden = true;
        byId('connectWifi').disabled = true;
      } else if (s.network.connected && !s.network.connecting) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = 'Router Wi-Fi connected! Select "Next" to open the dashboard.';
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden = false;
        byId('connectWifi').disabled = false;
      } else if (s.network.connecting) {
        byId('wifiMessage').textContent = 'Connecting to router Wi-Fi…';
        byId('wifiMessage').classList.remove('error');
      } else {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = s.network.authFailed
          ? 'Wi-Fi authentication failed. Check password.'
          : 'Could not connect. Ensure network is 2.4 GHz.';
        byId('wifiMessage').classList.add('error');
        byId('connectWifi').disabled = false;
      }
    } else if (!dashboardReady && s.network.setupRequired) {
      byId('wifiSetup').hidden = false;
      byId('dashboardContent').hidden = true;
      byId('savebar').hidden = true;
    } else if (!dashboardReady) {
      showDashboard();
    }

    // Hero Next Prayer Display
    if (s.nextPrayer && s.nextPrayer.name) {
      put('nextPrayerNameDisplay', `${s.nextPrayer.name.toUpperCase()} PRAYER`);
      put('nextPrayer', `${s.nextPrayer.name} at ${s.nextPrayer.time} (${s.nextPrayer.countdown})`);
    } else {
      put('nextPrayerNameDisplay', 'PRAYER SCHEDULE');
      put('nextPrayer', 'Schedule ready');
    }

    put('azanStatus', s.azanPlaying ? '🔔 Beeping — touch or shake to stop' : 'Ready (Armed)');
    put('hijri', s.hijri.date || '—');
    put('event', s.hijri.event || 'None today');

    // Populate Daily Prayer Times Cards
    const prayerNames = ['fajr', 'sunrise', 'dhuhr', 'asr', 'maghrib', 'isha'];
    for (const name of prayerNames) {
      const val = s.prayers[name];
      put(name, (val && val !== '00:00') ? val : '—');
      const card = byId(`card-${name}`);
      if (card) {
        const isNext = s.nextPrayer && s.nextPrayer.name && s.nextPrayer.name.toLowerCase() === name;
        card.classList.toggle('active-prayer', Boolean(isNext));
      }
    }
    put('prayerApiStatus', s.prayers.liveSynced ? 'Live Aladhan API ✅' : 'Default / Cache');

    // Populate Prohibited Times Cards
    const prohibitedWindows = [
      ['afterSunrise', 'afterSunrise'],
      ['atZawal', 'atZawal'],
      ['beforeSunset', 'beforeSunset']
    ];
    let anyProhibitedActive = false;
    let activeProhibitedName = '';
    let activeProhibitedEnd = '';

    for (const [key, id] of prohibitedWindows) {
      const window = s.prohibited?.[key];
      const valid = window && window.start && window.start !== '00:00' && window.end && window.end !== '00:00';
      put(`${id}Range`, valid ? `${window.start} – ${window.end}` : '—');
      const state = byId(`${id}State`);
      if (state) {
        state.textContent = valid ? (window.active ? '⚠️ Prohibited Now' : 'Not active now') : 'Schedule calculated';
        state.classList.toggle('prohibited-active', Boolean(valid && window?.active));
      }
      const pCard = byId(`card-prohibited-${key === 'afterSunrise' ? 'sunrise' : (key === 'atZawal' ? 'zawal' : 'sunset')}`);
      if (pCard) {
        pCard.classList.toggle('active-window', Boolean(valid && window?.active));
      }
      if (valid && window?.active) {
        anyProhibitedActive = true;
        activeProhibitedName = key === 'afterSunrise' ? 'After Sunrise' : (key === 'atZawal' ? 'At Zawal' : 'Before Sunset');
        activeProhibitedEnd = window.end;
      }
    }

    const pBanner = byId('prohibitedStatusBanner');
    const pText = byId('prohibitedBannerText');
    if (pBanner && pText) {
      if (anyProhibitedActive) {
        pBanner.className = 'prohibited-banner active-danger';
        pText.textContent = `⚠️ Prohibited Window Active: ${activeProhibitedName}. Voluntary Salat (Nafl) is forbidden until ${activeProhibitedEnd}.`;
      } else {
        pBanner.className = 'prohibited-banner';
        pText.textContent = 'Permitted Window: Safe to perform voluntary prayers (Nafl) and recite Quran.';
      }
    }

    // Pomodoro & Dhikr status
    put('pomodoroStatus', `${s.pomodoro.phase} · ${s.pomodoro.remaining} (Cycle ${s.pomodoro.cycle}/${s.pomodoro.cycles})`);
    put('dhikrStatus', `Count: ${s.dhikrCount}`);

    // Quran playback status
    if (s.quran) {
      const qNum = s.quran.surahNumber || '';
      const qName = s.quran.surah || 'None';
      put('quranStatus', `Surah ${qName} · ${s.quran.status || 'Idle'}`);
      put('quranNowPlaying', qNum ? `${qNum}. ${qName}` : qName);
      const phoneAudio = byId('phoneQuranAudio');
      const phonePlaying = byId('quranOutput').value === 'phone' && phoneAudio && !phoneAudio.paused && Boolean(phoneAudio.currentSrc);
      put('quranStreamStatus', phonePlaying ? 'Playing on this phone' : (s.quran.playing ? 'Playing on Prayer-Bot' : (s.quran.status || 'Idle')));
      const qSel = byId('quranSurahSelect');
      if (qSel && s.quran.surahNumber && document.activeElement !== qSel) {
        qSel.value = s.quran.surahNumber;
      }
    }

    // Weather status
    put('weatherCity', s.weather.city || 'Dhaka');
    put('temperature', s.weather.valid ? `${s.weather.temperatureC} °C` : 'Unavailable');
    put('humidity', s.weather.valid ? `${s.weather.humidity}%` : '—');
    put('aqi', s.weather.valid ? String(s.weather.aqi) : '—');
    put('pm25', s.weather.valid ? `${s.weather.pm25} µg/m³` : '—');

    if (initialStatus) {
      byId('screenSelect').value = s.screen;
      initialStatus = false;
    }
    return Boolean(s.prayers.liveSynced);
  }

  async function triggerPrayerSync() {
    const btn = byId('syncPrayerBtn');
    if (btn) btn.disabled = true;
    message('Syncing prayer & prohibited schedule with Aladhan API…');
    try {
      const res = await api('/api/prayer/refresh', { method: 'POST' });
      await loadStatus();
      message('Prayer times & prohibited schedule synced successfully.');
    } catch(e) {
      // Direct client-side fetch from Aladhan API if internet is available
      try {
        const lat = Number(byId('lat').value) || 23.8103;
        const lon = Number(byId('lon').value) || 90.4125;
        const method = Number(byId('calcMethod').value) || 3;
        const school = Number(byId('asrSchool').value) || 1;
        const res = await fetch(`https://api.aladhan.com/v1/timings?latitude=${lat}&longitude=${lon}&method=${method}&school=${school}`);
        const data = await res.json();
        if (data && data.data && data.data.timings) {
          const t = data.data.timings;
          put('fajr', t.Fajr); put('sunrise', t.Sunrise); put('dhuhr', t.Dhuhr);
          put('asr', t.Asr); put('maghrib', t.Maghrib); put('isha', t.Isha);
          put('prayerApiStatus', 'Aladhan Direct ✅');
          message('Prayer times updated directly from Aladhan online service.');
        } else {
          throw new Error('Aladhan response invalid');
        }
      } catch(onlineErr) {
        message(`Sync request: ${e.message}`, true);
      }
    } finally {
      if (btn) btn.disabled = false;
    }
  }

  function applyLocationPreset() {
    const location = locations[byId('locationPreset').value];
    if (!location) return;
    byId('city').value = location.city;
    byId('lat').value = location.lat;
    byId('lon').value = location.lon;
    byId('timezone').value = location.tz;
    byId('utcOffset').value = location.utcOff;
  }

  ['city', 'lat', 'lon', 'timezone', 'utcOffset'].forEach(id => {
    byId(id)?.addEventListener('input', () => { byId('locationPreset').value = 'custom'; });
  });

  function drawWeeklyChart(days) {
    const canvas = byId('weeklyChart');
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    if (!rect.width || !rect.height) return;
    const ratio = window.devicePixelRatio || 1;
    canvas.width = Math.round(rect.width * ratio);
    canvas.height = Math.round(rect.height * ratio);
    const ctx = canvas.getContext('2d');
    ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
    const w = rect.width, h = rect.height, pad = { l: 38, r: 12, t: 16, b: 32 };
    ctx.clearRect(0, 0, w, h);
    const maxMinutes = Math.max(60, ...days.map(d => d.focusMinutes));
    const maxSessions = Math.max(1, ...days.map(d => d.started));
    const chartW = w - pad.l - pad.r, chartH = h - pad.t - pad.b, step = chartW / 7;
    ctx.font = '12px system-ui';
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    for (let tick = 0; tick <= 3; tick++) {
      const y = pad.t + chartH * tick / 3;
      const minutes = Math.round(maxMinutes * (1 - tick / 3));
      ctx.strokeStyle = '#e2e8f0';
      ctx.beginPath();
      ctx.moveTo(pad.l, y);
      ctx.lineTo(w - pad.r, y);
      ctx.stroke();
      ctx.fillStyle = '#64748b';
      ctx.fillText(`${minutes}m`, pad.l - 6, y);
    }
    ctx.textAlign = 'center';
    ctx.textBaseline = 'top';
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const barH = chartH * day.focusMinutes / maxMinutes;
      ctx.fillStyle = '#0f766e';
      ctx.globalAlpha = 0.8;
      ctx.fillRect(x - step * 0.24, pad.t + chartH - barH, step * 0.48, barH);
      ctx.globalAlpha = 1;
      ctx.fillStyle = '#475569';
      ctx.fillText(day.weekday, x, h - pad.b + 8);
    });
    ctx.strokeStyle = '#d97706';
    ctx.lineWidth = 2.5;
    ctx.beginPath();
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const y = pad.t + chartH - chartH * day.started / Math.max(maxSessions, 4);
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    });
    ctx.stroke();
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const y = pad.t + chartH - chartH * day.started / Math.max(maxSessions, 4);
      ctx.fillStyle = '#d97706';
      ctx.beginPath();
      ctx.arc(x, y, 3.5, 0, Math.PI * 2);
      ctx.fill();
    });
  }

  async function loadWeeklyStats() {
    try {
      const stats = await api('/api/stats');
      if (!Array.isArray(stats.week) || stats.week.length !== 7) throw new Error('Invalid statistics');
      put('weeklyStarted', stats.totalStarted);
      put('weeklyCompleted', stats.totalCompleted);
      put('weeklyFocus', `${stats.totalFocusMinutes} min`);
      drawWeeklyChart(stats.week);
    } catch(e) {}
  }

  async function refreshAll() {
    try {
      await Promise.all([loadSettings(), loadStatus(), syncDeviceClock()]);
    } catch(e) {
      message(e.message, true);
    }
  }

  async function saveSettings() {
    const body = {
      city: byId('city').value.trim(),
      lat: Number(byId('lat').value),
      lon: Number(byId('lon').value),
      timezone: byId('timezone').value.trim(),
      utcOff: Number(byId('utcOffset').value),
      calcMethod: Number(byId('calcMethod').value),
      asrSchool: Number(byId('asrSchool').value),
      hijriOffset: Number(byId('hijriOffset').value || 0),
      azanOn: byId('azanOn').checked,
      volume: Number(byId('volume').value),
      focusMin: Number(byId('focusMin').value),
      shortBrk: Number(byId('shortBrk').value),
      longBrk: Number(byId('longBrk').value),
      pomCycles: Number(byId('pomCycles').value),
      sunriseOff: Number(byId('sunriseOff').value),
      zawalOff: Number(byId('zawalOff').value),
      sunsetOff: Number(byId('sunsetOff').value)
    };
    const owm = byId('owmKey')?.value.trim();
    if (owm) body.owmKey = owm;
    const gemini = byId('geminiKey')?.value.trim();
    if (gemini) body.geminiKey = gemini;

    const inputs = [...document.querySelectorAll('#city,#timezone,#lat,#lon,#utcOffset,#hijriOffset,#focusMin,#shortBrk,#longBrk,#pomCycles,#sunriseOff,#zawalOff,#sunsetOff')];
    const invalid = inputs.find(i => !i.checkValidity());
    if (invalid) {
      byId('settings').open = true;
      invalid.scrollIntoView({ behavior: 'smooth', block: 'center' });
      invalid.focus();
      invalid.reportValidity();
      message(`Check the ${invalid.labels[0]?.textContent || invalid.id} field.`, true);
      return;
    }

    const buttons = [...document.querySelectorAll('#saveButton, #settings button.primary')];
    buttons.forEach(b => { b.disabled = true; b.textContent = 'Saving…'; });

    try {
      await api('/api/settings', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });
      if (byId('owmKey')) byId('owmKey').value = '';
      if (byId('geminiKey')) byId('geminiKey').value = '';
      await loadSettings();
      await loadStatus();
      message('Settings saved to device storage and applied.');
    } catch(e) {
      message(e.message, true);
    } finally {
      buttons.forEach(b => { b.disabled = false; b.textContent = '💾 Save & Apply All Settings'; });
    }
  }

  async function sendControl(body) {
    try {
      await api('/api/control', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });
      message('Command sent to device.');
      await loadStatus();
    } catch(e) {
      message(e.message, true);
    }
  }

  const surahs = [
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
    const sel = byId('quranSurahSelect');
    if (!sel) return;
    sel.replaceChildren();
    surahs.forEach((name, i) => {
      sel.add(new Option(`${i + 1}. ${name}`, i + 1));
    });
    sel.value = 1;
  }

  let browserRecognition = null;
  let voiceAttemptGeneration = 0;
  function setBrowserVoiceState(listening, text, error = false) {
    byId('voiceOrb')?.classList.toggle('listening', listening);
    byId('browserVoiceButton').disabled = listening || !routerWifiConnected;
    byId('browserVoiceCancel').hidden = !listening;
    put('browserVoiceStatus', text);
    byId('browserVoiceStatus').style.color = error ? 'var(--bad)' : 'var(--text)';
  }

  function normalizeSurahSpeech(value) {
    return value.toLowerCase().replace(/[^\p{L}\p{N}]+/gu, ' ').trim().replace(/\s+/g, ' ');
  }

  function findSpokenSurah(transcript) {
    const spoken = normalizeSurahSpeech(transcript);
    const padded = ` ${spoken} `;
    for (let i = 0; i < surahs.length; i++) {
      const name = normalizeSurahSpeech(surahs[i]);
      if (padded.includes(` ${name} `)) return i + 1;
    }
    const aliases = {
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
    for (const [name, number] of Object.entries(aliases)) {
      if (padded.includes(` ${normalizeSurahSpeech(name)} `)) return number;
    }
    return 0;
  }

  async function chooseSurah(number) {
    await ensureRouterConnection();
    if (!Number.isInteger(number) || number < 1 || number > surahs.length) {
      throw new Error('Could not identify a Surah. Try again or pick from the list.');
    }
    byId('quranSurahSelect').value = number;
    const name = surahs[number - 1];
    await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'select', surah: number }) });
    put('quranNowPlaying', `${number}. ${name}`);

    if (byId('quranOutput').value === 'phone') {
      await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'stop' }) });
      const audio = byId('phoneQuranAudio');
      audio.dataset.fallbackUsed = 'false';
      audio.onerror = () => {
        if (audio.dataset.fallbackUsed === 'false') {
          audio.dataset.fallbackUsed = 'true';
          audio.src = `https://cdn.islamic.network/quran/audio/128/ar.alafasy/${number}.mp3`;
          put('quranStreamStatus', 'Primary server busy; trying alternate…');
          audio.play().catch(() => put('quranStreamStatus', 'Tap play on phone controls.'));
        } else {
          put('quranStreamStatus', 'Recitation could not load. Check network.');
        }
      };
      audio.src = `https://cdn.mp3quran.net/audio/maher-muaiqly/r1/${String(number).padStart(3, '0')}.mp3`;
      audio.style.display = 'block';
      put('quranStreamStatus', 'Loading recitation on this phone…');
      try {
        await audio.play();
        put('quranStreamStatus', 'Playing on this phone');
      } catch(e) {
        put('quranStreamStatus', 'Tap play on audio controls to start.');
      }
      return;
    }

    byId('phoneQuranAudio').pause();
    const response = await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'play', surah: number }) });
    put('quranStreamStatus', response.status === 'queued' ? 'Connecting to device speaker…' : 'Starting recitation…');
    await loadStatus();
  }

  async function recognizeTranscriptWithGemini(transcript) {
    const queued = await api('/api/quran/recognize', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ transcript })
    });
    if (!Number.isInteger(queued.requestId) || queued.requestId < 1) {
      throw new Error('Device did not return a valid Gemini recognition ID.');
    }
    for (let attempt = 0; attempt < 60; attempt++) {
      await new Promise(r => setTimeout(r, 750));
      const res = await api(`/api/quran/recognize?id=${encodeURIComponent(queued.requestId)}`);
      if (res.status === 'done' && Number.isInteger(res.surahNumber)) return res.surahNumber;
      if (res.status === 'error') throw new Error(res.error || 'Gemini could not identify Surah.');
      if (res.status === 'unknown') throw new Error('Recognition request expired.');
      if (attempt % 4 === 0) setBrowserVoiceState(true, 'Gemini is identifying the Surah name…');
    }
    throw new Error('Gemini recognition timed out.');
  }

  let processingSpokenResult = false;
  async function listenForSurah() {
    const attempt = ++voiceAttemptGeneration;
    setBrowserVoiceState(true, 'Checking router connection…');
    try {
      await ensureRouterConnection();
    } catch(e) {
      if (attempt !== voiceAttemptGeneration) return;
      setBrowserVoiceState(false, e.message, true);
      return;
    }
    const Recognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!Recognition) {
      if (attempt !== voiceAttemptGeneration) return;
      setBrowserVoiceState(false, 'Speech recognition not supported in this browser.', true);
      return;
    }
    if (browserRecognition) browserRecognition.abort();
    browserRecognition = new Recognition();
    browserRecognition.lang = byId('voiceLanguage').value;
    browserRecognition.continuous = false;
    browserRecognition.interimResults = true;
    browserRecognition.onstart = () => {
      if (attempt === voiceAttemptGeneration) setBrowserVoiceState(true, 'Listening — say a Surah name…');
    };
    browserRecognition.onresult = async event => {
      if (attempt !== voiceAttemptGeneration) return;
      const result = event.results[event.results.length - 1];
      const transcript = result[0].transcript;
      setBrowserVoiceState(true, `I heard: “${transcript}”`);
      if (!result.isFinal) return;
      processingSpokenResult = true;
      setBrowserVoiceState(true, 'Identifying Surah…');
      let number = 0, fallbackUsed = false;
      try {
        try {
          number = await recognizeTranscriptWithGemini(transcript);
        } catch(geminiErr) {
          if (attempt !== voiceAttemptGeneration) return;
          number = findSpokenSurah(transcript);
          if (!number) throw geminiErr;
          fallbackUsed = true;
        }
        if (attempt !== voiceAttemptGeneration) return;
        setBrowserVoiceState(false, `${fallbackUsed ? 'Local match' : 'Gemini recognized'}: Surah ${number}, ${surahs[number - 1]}`);
        await chooseSurah(number);
      } catch(e) {
        if (attempt === voiceAttemptGeneration) setBrowserVoiceState(false, e.message, true);
      } finally {
        if (attempt === voiceAttemptGeneration) processingSpokenResult = false;
      }
    };
    browserRecognition.onerror = event => {
      if (attempt !== voiceAttemptGeneration) return;
      processingSpokenResult = false;
      const reason = event.error === 'not-allowed' ? 'Allow microphone permission and try again.' : `Speech error (${event.error}).`;
      setBrowserVoiceState(false, reason, true);
    };
    browserRecognition.onend = () => {
      if (attempt === voiceAttemptGeneration && !processingSpokenResult && byId('voiceOrb').classList.contains('listening')) {
        setBrowserVoiceState(false, 'No Surah name detected. Try again.', true);
      }
    };
    try {
      browserRecognition.start();
    } catch(e) {
      setBrowserVoiceState(false, 'Could not start microphone.', true);
    }
  }

  function cancelBrowserVoice() {
    ++voiceAttemptGeneration;
    processingSpokenResult = false;
    if (browserRecognition) browserRecognition.abort();
    browserRecognition = null;
    setBrowserVoiceState(false, 'Voice input cancelled.');
  }

  async function playQuran() {
    const sel = byId('quranSurahSelect');
    const surah = Number(sel?.value) || 1;
    try {
      await chooseSurah(surah);
      message(`Playing Surah ${surah}: ${surahs[surah - 1] || ''}.`);
    } catch(e) {
      message(e.message, true);
    }
  }

  async function stopQuran() {
    try {
      const audio = byId('phoneQuranAudio');
      audio.pause();
      audio.dataset.fallbackUsed = 'false';
      await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'stop' }) });
      message('Quran recitation stopped.');
      await loadStatus();
    } catch(e) {
      message(e.message, true);
    }
  }

  async function nextQuran() {
    try {
      const next = (Number(byId('quranSurahSelect').value) % surahs.length) + 1;
      if (byId('quranOutput').value === 'phone') await chooseSurah(next);
      else await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'next' }) });
      await loadStatus();
    } catch(e) {
      message(e.message, true);
    }
  }

  async function prevQuran() {
    try {
      const current = Number(byId('quranSurahSelect').value) || 1;
      const prev = current <= 1 ? surahs.length : current - 1;
      if (byId('quranOutput').value === 'phone') await chooseSurah(prev);
      else await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'prev' }) });
      await loadStatus();
    } catch(e) {
      message(e.message, true);
    }
  }

  async function saveWifi() {
    const ssid = byId('wifiSsid').value.trim();
    const password = byId('wifiPass').value;
    if (!ssid) {
      byId('wifiMessage').textContent = 'Please enter a Wi-Fi network name (SSID).';
      byId('wifiMessage').classList.add('error');
      return;
    }
    if (password && (password.length < 8 || password.length > 63)) {
      byId('wifiMessage').textContent = 'Wi-Fi password must be 8 to 63 characters (or blank for an open network).';
      byId('wifiMessage').classList.add('error');
      byId('wifiPass').focus();
      return;
    }
    byId('connectWifi').disabled = true;
    byId('continueWifi').hidden = true;
    try {
      await api('/api/wifi', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ ssid, pass: password }) });
      wifiSubmitted = true;
      byId('wifiMessage').textContent = 'Connecting to Wi-Fi router… waiting for verification.';
      byId('wifiMessage').classList.remove('error');
    } catch(e) {
      byId('wifiMessage').textContent = e.message;
      byId('wifiMessage').classList.add('error');
      byId('connectWifi').disabled = false;
    }
  }

  async function scanWifiNetworks() {
    const button = byId('scanWifi'), sel = byId('wifiNetworks'), msg = byId('wifiMessage');
    button.disabled = true;
    sel.disabled = true;
    sel.replaceChildren(new Option('⏳ Scanning for nearby 2.4 GHz Wi-Fi…', ''));
    msg.textContent = 'Scanning for nearby networks — please wait…';
    msg.classList.remove('error');
    try {
      let postOk = false;
      for (let t = 0; t < 5; t++) {
        try {
          await api('/api/wifi/scan', { method: 'POST' });
          postOk = true;
          break;
        } catch(e) {
          if (t < 4) await new Promise(r => setTimeout(r, 1200));
          else throw e;
        }
      }
      if (!postOk) throw new Error('Could not start Wi-Fi scan.');
      let result;
      for (let attempt = 0; attempt < 20; attempt++) {
        await new Promise(r => setTimeout(r, 1000));
        result = await api('/api/wifi/scan');
        if (result.status === 'complete') break;
        const dots = '.'.repeat((attempt % 3) + 1);
        sel.options[0].text = `⏳ Scanning${dots}`;
      }
      if (!result || result.status !== 'complete') throw new Error('Scan timed out.');
      if (!Array.isArray(result.networks)) throw new Error('Invalid scan result.');
      const strongest = new Map();
      for (const n of result.networks) {
        if (!n.ssid) continue;
        const prev = strongest.get(n.ssid);
        if (!prev || Number(n.rssi) > Number(prev.rssi)) strongest.set(n.ssid, n);
      }
      const sorted = [...strongest.values()].sort((a, b) => Number(b.rssi) - Number(a.rssi));
      sel.replaceChildren(new Option('— Select your home Wi-Fi —', ''));
      for (const n of sorted) {
        const bars = n.rssi >= -60 ? '🟢 ▂▄▆█' : n.rssi >= -75 ? '🟡 ▂▄▆_' : '🟠 ▂▄__';
        sel.add(new Option(`${bars} ${n.ssid} (${n.rssi} dBm${n.secured ? ' · 🔒' : ''})`, n.ssid));
      }
      msg.textContent = sel.options.length > 1 ? `Found ${sel.options.length - 1} network(s). Select one and enter password.` : 'No networks found. Try scanning again.';
    } catch(e) {
      sel.replaceChildren(new Option('— tap Scan to discover networks —', ''));
      msg.textContent = e.message;
      msg.classList.add('error');
    } finally {
      button.disabled = false;
      sel.disabled = false;
    }
  }

  byId('wifiNetworks').addEventListener('change', event => {
    if (event.target.value) {
      byId('wifiSsid').value = event.target.value;
      byId('wifiPass').value = '';
      byId('wifiPass').focus();
      byId('wifiMessage').textContent = 'Network selected. Enter router password and tap Connect.';
      byId('wifiMessage').classList.remove('error');
    }
  });

  initQuranDropdown();
  refreshAll();
  loadWeeklyStats();

  // Adaptive polling: fast (3s) until live prayer sync, then regular (5s)
  let prayerReady = false;
  async function pollStatus() {
    try { prayerReady = await loadStatus(); } catch(e) {}
    setTimeout(pollStatus, prayerReady ? 5000 : 3000);
  }
  setTimeout(pollStatus, 3000);
  setInterval(() => loadWeeklyStats(), 30000);
  setInterval(() => {
    if (wifiSubmitted) loadStatus().catch(() => {
      byId('wifiMessage').textContent = 'Prayer-Bot is connecting to router… check router IP shortly.';
    });
  }, 2000);
  window.addEventListener('resize', () => loadWeeklyStats());
</script>
</body>
</html>
)rawliteral";

#endif
