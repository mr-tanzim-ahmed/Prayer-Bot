#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <meta name="theme-color" content="#065f46">
  <title>Prayer-Bot | Mumins Personal Assistant</title>
<link rel="icon" type="image/svg+xml" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><circle cx='50' cy='50' r='40' fill='%230f766e'/></svg>">
<link rel="apple-touch-icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><circle cx='50' cy='50' r='40' fill='%230f766e'/></svg>">
  <style>
    :root {
      --primary: #0f806f;
      --primary-dark: #075448;
      --primary-hover: #0d927d;
      --primary-soft: #e8f7f1;
      --accent: #c47b20;
      --accent-soft: #fff4df;
      --bg: #f4f8f6;
      --panel: #ffffff;
      --panel-subtle: #f6faf8;
      --line: #dfeae5;
      --line-hover: #b7d2c7;
      --text: #142a25;
      --text-muted: #60766e;
      --warn: #a85b12;
      --warn-soft: #fff6e8;
      --bad: #c03935;
      --bad-soft: #fff0ef;
      --good: #15805c;
      --good-soft: #eaf8f0;
      --radius: 18px;
      --radius-sm: 12px;
      --radius-lg: 26px;
      --shadow-sm: 0 2px 8px rgba(20,54,43,0.05);
      --shadow: 0 8px 26px rgba(20,70,53,0.065);
      --shadow-lg: 0 18px 48px rgba(20,54,43,0.14);
      --ease-out: cubic-bezier(0.2, 0.75, 0.25, 1);
    }
    * { box-sizing: border-box; }
    html { scroll-behavior: smooth; }
    body {
      margin: 0;
      background:
        radial-gradient(ellipse at 8% 0%, rgba(181, 229, 208, 0.36), transparent 34rem),
        radial-gradient(ellipse at 96% 18%, rgba(244, 218, 172, 0.2), transparent 32rem),
        linear-gradient(180deg, #f8fbf9 0%, var(--bg) 42rem, #f5f8f7 100%);
      color: var(--text);
      font: 15px/1.55 Inter, system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
      -webkit-font-smoothing: antialiased;
      text-rendering: optimizeLegibility;
    }
    a { color: var(--primary); text-decoration: none; }
    a:hover { text-decoration: underline; }
    
    /* Top Sticky App Bar */
    .app-bar {
      position: sticky;
      top: 0;
      z-index: 100;
      background: rgba(248, 252, 250, 0.86);
      backdrop-filter: blur(20px) saturate(155%);
      -webkit-backdrop-filter: blur(20px) saturate(155%);
      border-bottom: 1px solid rgba(200, 222, 211, 0.76);
      box-shadow: 0 4px 20px rgba(20, 54, 43, 0.045);
      transition: background 0.28s var(--ease-out), box-shadow 0.28s var(--ease-out);
    }
    .app-bar.is-scrolled {
      background: rgba(248, 252, 250, 0.94);
      -webkit-backdrop-filter: blur(24px) saturate(185%);
      backdrop-filter: blur(24px) saturate(185%);
      box-shadow: 0 10px 28px rgba(20, 54, 43, 0.1);
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
      gap: 10px;
    }
    .brand-icon {
      width: 42px;
      height: 42px;
      border-radius: 12px;
      background:
        linear-gradient(145deg, rgba(255,255,255,.2), transparent 58%),
        linear-gradient(135deg, #064a40, #15917a);
      color: #fff;
      display: grid;
      place-items: center;
      box-shadow: 0 5px 14px rgba(7, 84, 72, 0.24), inset 0 1px 0 rgba(255,255,255,.28);
    }
    .brand-icon svg { width: 24px; height: 24px; fill: currentColor; }
    .brand-title {
      font-size: 1.35rem;
      font-weight: 750;
      letter-spacing: -0.03em;
      color: #075448;
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
      background: rgba(255,255,255,.68);
      border: 1px solid rgba(213, 230, 221, .9);
      color: #38564b;
      font-size: 0.84rem;
      font-weight: 600;
      white-space: nowrap;
      transition: color .22s var(--ease-out), background .22s var(--ease-out), border-color .22s var(--ease-out), box-shadow .22s var(--ease-out), transform .22s var(--ease-out);
      cursor: pointer;
    }
    .nav-pill:hover, .nav-pill:focus-visible {
      background: #e8f7f1;
      border-color: #9dcdb8;
      color: var(--primary-dark);
      transform: translateY(-1px);
      text-decoration: none;
    }
    .nav-pill.active {
      background: linear-gradient(135deg, #138773, #075448);
      color: #fff;
      border-color: transparent;
      box-shadow: 0 4px 12px rgba(7, 84, 72, .2), inset 0 1px 0 rgba(255,255,255,.22);
    }
    
    /* Layout */
    main {
      max-width: 1140px;
      margin: auto;
      padding: 30px 20px 88px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(12, minmax(0, 1fr));
      gap: 16px;
    }
    .card {
      min-width: 0;
      background: var(--panel);
      border: 1px solid rgba(219, 232, 225, .92);
      border-radius: var(--radius);
      padding: 20px;
      box-shadow: var(--shadow);
      position: relative;
      transition: border-color .28s var(--ease-out), box-shadow .28s var(--ease-out), transform .28s var(--ease-out);
    }
    @media (hover: hover) and (prefers-reduced-motion: no-preference) {
      .grid > .card:not(.hero-banner):hover {
        border-color: #c8ddd2;
        box-shadow: 0 14px 34px rgba(20, 70, 53, .09);
        transform: translateY(-2px);
      }
    }
    .wide { grid-column: span 12; }
    .grid > .card:not(.wide) { grid-column: span 6; }
    [id] { scroll-margin-top: 148px; }
    
    /* Typography & Headers */
    h2 {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 1.15rem;
      font-weight: 700;
      letter-spacing: -0.02em;
      margin: 0 0 12px;
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
      background:
        radial-gradient(ellipse at 88% 0%, rgba(230, 188, 111, .24), transparent 34%),
        radial-gradient(ellipse at 8% 100%, rgba(37, 173, 137, .2), transparent 42%),
        linear-gradient(120deg, #073b35 0%, #0b6256 52%, #137564 100%);
      color: #fff;
      border: none;
      padding: 18px 20px;
      box-shadow: 0 18px 42px rgba(7, 70, 57, 0.22), inset 0 1px 0 rgba(255,255,255,.16);
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
    .hero-banner::before {
      content: "";
      position: absolute;
      inset: 0;
      background: linear-gradient(115deg, transparent 38%, rgba(255,255,255,.045) 58%, transparent 78%);
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
      color: #b4ead4;
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
      color: #ffe2a2;
    }
    .hero-stats {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 8px;
      background: rgba(4, 43, 38, 0.28);
      border: 1px solid rgba(255, 255, 255, 0.18);
      border-radius: 14px;
      padding: 18px;
      box-shadow: inset 0 1px 0 rgba(255,255,255,.08);
    }
    .hero-stat span { display: block; font-size: 0.72rem; color: #b4ead4; font-weight: 650; text-transform: uppercase; letter-spacing: .045em; }
    .hero-stat strong { display: block; font-size: 1.05rem; font-weight: 700; color: #fff; margin-top: 2px; }
    
    /* Prayer Cards Grid */
    .prayers-grid {
      display: grid;
      grid-template-columns: repeat(6, minmax(0, 1fr));
      gap: 12px;
      margin-top: 8px;
    }
    .prayer-card {
      background: linear-gradient(155deg, #fff 0%, #f5faf7 100%);
      border: 1px solid var(--line);
      border-radius: 14px;
      padding: 12px 8px;
      text-align: center;
      transition: transform .24s var(--ease-out), box-shadow .24s var(--ease-out), border-color .24s var(--ease-out), background .24s var(--ease-out);
      position: relative;
      overflow: hidden;
    }
    .prayer-card:hover {
      transform: translateY(-3px);
      box-shadow: 0 10px 22px rgba(15,118,110,0.11);
      border-color: var(--primary);
    }
    .prayer-card.active-prayer {
      background: linear-gradient(150deg, #eaf8f1 0%, #dff3ea 100%);
      border-color: #66ae91;
      box-shadow: 0 5px 18px rgba(15, 118, 110, 0.1), inset 0 0 0 1px rgba(255,255,255,.72);
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
    .fasting-times {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
      margin-top: 16px;
    }
    .fasting-time {
      padding: 14px 16px;
      border: 1.5px solid var(--line);
      border-radius: 14px;
      background: var(--panel-subtle);
      text-align: center;
    }
    .fasting-time span {
      display: block;
      color: var(--text-muted);
      font-size: 0.82rem;
      font-weight: 700;
    }
    .fasting-time strong {
      display: block;
      margin-top: 4px;
      color: var(--primary-dark);
      font-size: 1.35rem;
      font-variant-numeric: tabular-nums;
    }
    @media (max-width: 480px) {
      .fasting-times { grid-template-columns: 1fr; }
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
      gap: 8px;
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
      gap: 10px;
    }
    .prohibited-card {
      background: var(--panel-subtle);
      border: 1.5px solid var(--line);
      border-radius: 14px;
      padding: 12px;
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
      gap: 10px;
    }
    .stat {
      padding: 12px;
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
      transition: border-color .2s var(--ease-out), box-shadow .2s var(--ease-out), background .2s var(--ease-out);
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
      position: relative;
      overflow: hidden;
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
      transition: background 0.18s, border-color 0.18s, transform 0.12s, box-shadow 0.18s;
    }
    button:hover {
      background: var(--panel-subtle);
      border-color: var(--line-hover);
    }
    button:active { transform: translateY(1px) scale(0.985); }
    button.primary {
      background: linear-gradient(180deg, rgba(255,255,255,.24), transparent 58%),
                  linear-gradient(135deg, #15947e, #075448);
      color: #ffffff;
      border-color: rgba(255,255,255,0.28);
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.32),
                  0 7px 18px rgba(15, 118, 110, 0.24);
    }
    button.primary:hover {
      background: linear-gradient(180deg, rgba(255,255,255,.3), transparent 58%),
                  linear-gradient(135deg, #20a58e, #086452);
      border-color: var(--primary-hover);
      box-shadow: inset 0 1px 0 rgba(255,255,255,.38), 0 9px 22px rgba(15, 118, 110, .28);
    }
    button.accent {
      background: linear-gradient(180deg, rgba(255,255,255,0.28), transparent 48%),
                  linear-gradient(135deg, #f59e0b, #b45309);
      color: #fff;
      border-color: rgba(255,255,255,0.28);
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.32),
                  0 4px 12px rgba(180,83,9,0.2);
    }
    button.accent:hover {
      background: linear-gradient(135deg, #d18a2e, #a95512);
    }
    button:disabled { opacity: 0.55; cursor: wait; }
    button:disabled:active { transform: none; }
    .actions {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
    }
    .actions button { flex: 1; }
    .quran-actions {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 8px;
    }
    .quran-actions .quran-play { grid-column: 1 / -1; }
    .quran-actions button {
      min-width: 0;
      min-height: 44px;
      padding: 8px 6px;
      white-space: nowrap;
    }
    .section-reveal {
      opacity: 0;
      transform: translateY(12px);
      transition: opacity 0.5s ease, transform 0.5s ease;
      transition-delay: var(--reveal-delay, 0ms);
    }
    .section-reveal.is-visible {
      opacity: 1;
      transform: translateY(0);
    }
    
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
      background: linear-gradient(110deg, #edf9f2, #e5f6ee);
      border: 1px solid #c8e8d6;
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

    .wifi-scan-dialog {
      width: min(560px, calc(100% - 24px));
      max-height: min(78vh, 680px);
      padding: 0;
      border: 1px solid rgba(255,255,255,0.75);
      border-radius: 22px;
      color: var(--text);
      background: rgba(255,255,255,0.94);
      box-shadow: 0 24px 80px rgba(15,23,42,0.28);
      -webkit-backdrop-filter: blur(22px) saturate(1.4);
      backdrop-filter: blur(22px) saturate(1.4);
    }
    .wifi-scan-dialog::backdrop {
      background: rgba(15,23,42,0.42);
      -webkit-backdrop-filter: blur(5px);
      backdrop-filter: blur(5px);
    }
    .wifi-dialog-body { padding: 20px; }
    .wifi-dialog-header {
      display: flex;
      align-items: flex-start;
      justify-content: space-between;
      gap: 12px;
      margin-bottom: 14px;
    }
    .wifi-dialog-header h2 { margin: 0; }
    .wifi-dialog-header button { min-height: 42px; padding: 6px 12px; }
    .wifi-scan-status {
      min-height: 72px;
      display: flex;
      align-items: center;
      gap: 14px;
      padding: 14px;
      border: 1px solid var(--line);
      border-radius: 14px;
      background: rgba(241,245,249,0.8);
      color: var(--text-muted);
      font-weight: 650;
    }
    .wifi-scan-spinner {
      width: 28px;
      height: 28px;
      flex: 0 0 28px;
      border: 3px solid #cbd5e1;
      border-top-color: var(--primary);
      border-radius: 50%;
      animation: wifi-spin 0.8s linear infinite;
    }
    .wifi-scan-status[data-loading="false"] .wifi-scan-spinner { display: none; }
    @keyframes wifi-spin { to { transform: rotate(360deg); } }
    .wifi-scan-list {
      display: grid;
      gap: 8px;
      max-height: 48vh;
      overflow: auto;
      margin-top: 12px;
    }
    .wifi-network-choice {
      display: flex;
      justify-content: space-between;
      text-align: left;
      min-height: 56px;
      width: 100%;
    }
    .wifi-network-choice small { color: var(--text-muted); }
    
    /* Al-Quran Voice Panel & Visualizer */
    .voice-panel {
      display: grid;
      grid-template-columns: auto 1fr;
      align-items: center;
      gap: 16px;
      padding: 18px;
      background: linear-gradient(135deg, #eff9f3, #e4f6ed);
      border: 1px solid #c8e8d6;
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
    .voice-orb.connecting {
      color: #b7791f;
      border-color: #f6c453;
      animation: voice-pulse 1.4s ease-in-out infinite;
    }
    .voice-orb.recognizing {
      color: #6d5bd0;
      border-color: #a99cf4;
      background: #f0edff;
      animation: voice-pulse 0.8s ease-in-out infinite;
    }
    #browserVoiceButton { touch-action: none; user-select: none; }
    #browserVoiceButton.holding {
      background: var(--primary);
      color: #fff;
      border-color: var(--primary);
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
    .assistant-heading[data-voice-phase="listening"] .voice-bars i {
      animation: voice-wave 0.75s ease-in-out infinite alternate;
      opacity: 1;
    }
    .assistant-heading[data-voice-phase="recognizing"] .voice-bars i {
      background: #6d5bd0;
      animation: voice-wave 0.45s ease-in-out infinite alternate;
      opacity: 1;
    }
    .voice-bars i:nth-child(2n) { animation-delay: 0.15s; }
    .voice-bars i:nth-child(3n) { animation-delay: 0.3s; }
    @keyframes voice-pulse {
      50% { box-shadow: 0 0 0 10px rgba(15, 118, 110, 0.18); transform: scale(1.05); }
    }
    @keyframes voice-wave { to { height: 28px; } }
    .voice-activity {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 5px 9px;
      border: 1px solid #c8e8d6;
      border-radius: 999px;
      background: rgba(255,255,255,.82);
      color: var(--primary-dark);
      font-size: .78rem;
      font-weight: 750;
      font-variant-numeric: tabular-nums;
    }
    .voice-activity[hidden] { display: none; }
    .voice-activity[data-phase="recognizing"] {
      color: #5746b2;
      border-color: #d7d0ff;
    }
    .voice-activity[data-phase="connecting"] {
      color: #93600c;
      border-color: #f4dda9;
    }
    .voice-timer-icon {
      width: 16px;
      height: 16px;
      flex: 0 0 16px;
      animation: voice-timer-spin 2.4s linear infinite;
      transform-origin: center;
    }
    @keyframes voice-timer-spin { to { transform: rotate(360deg); } }
    @media (prefers-reduced-motion: reduce) {
      .voice-orb, .voice-timer-icon, .voice-bars i { animation: none !important; }
    }
    @media (max-width: 520px) {
      .assistant-panel { padding: 14px; }
      .assistant-search { grid-template-columns: 1fr; }
    }
    .voice-status { min-height: 24px; font-weight: 650; margin: 4px 0; color: var(--text); }
    .assistant-panel {
      display: grid;
      gap: 12px;
      padding: 18px;
      border: 1px solid rgba(176, 202, 189, .78);
      border-radius: 20px;
      background:
        radial-gradient(circle at 95% 0%, rgba(232, 194, 126, .28), transparent 34%),
        radial-gradient(circle at 4% 100%, rgba(115, 199, 157, .2), transparent 38%),
        linear-gradient(135deg, #f1f8f3, #edf7f2 54%, #fdf7e9);
      box-shadow: inset 0 1px 0 rgba(255,255,255,.78);
    }
    .assistant-heading { display: flex; align-items: center; gap: 14px; }
    .assistant-heading .voice-orb { flex: 0 0 auto; }
    .assistant-greeting { font-weight: 800; color: var(--primary-dark); }
    .assistant-examples { color: var(--text-muted); font-size: .86rem; line-height: 1.55; }
    .assistant-search { display: grid; grid-template-columns: 1fr auto; gap: 8px; }
    .assistant-search input { min-width: 0; }
    .assistant-result {
      min-height: 42px;
      padding: 10px 12px;
      border-radius: 12px;
      background: rgba(255, 255, 255, .82);
      color: var(--text);
      line-height: 1.45;
      border: 1px solid rgba(255,255,255,.78);
    }
    .assistant-result.error { color: var(--bad); }
    
    /* Pomodoro & Dhikr Counters */
    .dhikr-button {
      width: 100%;
      min-height: 64px;
      font-size: 1.15rem;
      font-weight: 800;
      border-radius: 16px;
      background: linear-gradient(135deg, #1a9b83 0%, #0b705f 52%, #064a40 100%);
      color: #fff;
      border: none;
      box-shadow: 0 8px 20px rgba(15, 118, 110, 0.24), inset 0 1px 0 rgba(255,255,255,.22);
      transition: transform .2s var(--ease-out), box-shadow .2s var(--ease-out), filter .2s var(--ease-out);
      -webkit-tap-highlight-color: transparent;
      touch-action: manipulation;
    }
    button.dhikr-button,
    button.dhikr-button:hover,
    button.dhikr-button:active,
    button.dhikr-button:focus-visible {
      background: linear-gradient(135deg, #1a9b83 0%, #0b705f 52%, #064a40 100%);
      color: #fff;
      border-color: transparent;
    }
    button.dhikr-button:active { transform: scale(0.985); }
    @media (hover: hover) {
      button.dhikr-button:hover {
        filter: brightness(1.08);
        box-shadow: 0 11px 26px rgba(15, 118, 110, .29), inset 0 1px 0 rgba(255,255,255,.25);
      }
    }
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
    .chart-scale-note {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      margin: 3px 4px -6px;
      color: var(--text-muted);
      font-size: .75rem;
      font-weight: 650;
    }
    .weekly-breakdown {
      display: grid;
      grid-template-columns: repeat(7, minmax(0, 1fr));
      gap: 8px;
      margin-top: 12px;
    }
    .weekly-day {
      min-width: 0;
      padding: 9px 5px;
      border: 1px solid var(--line);
      border-radius: 12px;
      background: linear-gradient(180deg, #fff, #f7fbf8);
      text-align: center;
    }
    .weekly-day strong, .weekly-day span { display: block; }
    .weekly-day strong { color: var(--text); font-size: .82rem; }
    .weekly-day span { color: var(--text-muted); font-size: .72rem; line-height: 1.5; }
    .weekly-day.today {
      border-color: rgba(15, 128, 111, .55);
      background: linear-gradient(180deg, #effaf5, #e3f5ec);
      box-shadow: inset 0 0 0 1px rgba(15, 128, 111, .08);
    }
    @media (max-width: 580px) {
      .weekly-breakdown { grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 6px; }
      .weekly-day { padding: 8px 3px; }
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
      bottom: max(12px, env(safe-area-inset-bottom));
      z-index: 90;
      margin: 12px auto 0;
      max-width: 560px;
      padding: 10px 12px;
      background: rgba(250, 253, 251, 0.9);
      backdrop-filter: blur(20px) saturate(1.45);
      -webkit-backdrop-filter: blur(20px) saturate(1.45);
      border: 1px solid rgba(209, 229, 218, 0.9);
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
      border: 1px solid #f3d9b8;
      border-radius: 12px;
      background: linear-gradient(110deg, #fff8ec, #fff3e7);
      color: #85430f;
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

    @media (hover: hover) and (prefers-reduced-motion: no-preference) {
      button:not(.primary):not(.accent):not(.dhikr-button):not(:disabled):hover {
        transform: translateY(-1px);
        box-shadow: 0 7px 18px rgba(20, 54, 43, .1);
      }
      button.primary:hover, button.accent:hover, .dhikr-button:hover {
        transform: translateY(-2px);
      }
    }

    /* Responsive Queries */
    @media (max-width: 960px) {
      .grid > .card:not(.wide) { grid-column: span 12; }
      .prayers-grid { grid-template-columns: repeat(3, minmax(0, 1fr)); }
      .prohibited-grid { grid-template-columns: 1fr; }
      .hero-grid { grid-template-columns: 1fr; gap: 16px; }
    }
    @media (max-width: 580px) {
      main { padding: 16px 12px max(60px, calc(60px + env(safe-area-inset-bottom))); }
      .app-bar-inner { padding: 10px 12px 8px; gap: 10px; flex-wrap: wrap; }
      .brand { gap: 9px; min-width: 0; }
      .brand-icon { width: 38px; height: 38px; flex: 0 0 38px; }
      .brand-title { font-size: 1.15rem; }
      .brand-subtitle { flex-wrap: wrap; gap: 4px; font-size: 0.72rem; }
      .app-bar-actions { width: 100%; }
      .app-bar-actions button { flex: 1; min-height: 44px; }
      .nav-bar { padding: 4px 12px 9px; gap: 6px; }
      .nav-pill { min-height: 38px; padding: 7px 11px; }
      .card { padding: 15px 12px; border-radius: 14px; }
      .fields { grid-template-columns: 1fr; }
      .fields > .field[style*="grid-column"] { grid-column: auto !important; }
      .prayers-grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
      .stats { grid-template-columns: repeat(2, minmax(0, 1fr)); }
      .actions { gap: 8px; }
      .actions button { flex: 1 1 100%; min-height: 48px; }
      .quran-actions { grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 6px; }
      .quran-actions button { min-height: 44px; padding: 7px 3px; font-size: 0.78rem; }
      .hero-banner { padding: 20px 16px; }
      .chart-wrap { height: 200px; }
      .settings > summary { padding: 17px 14px; gap: 10px; font-size: 1rem; }
      .settings > summary .badge { max-width: 46%; text-align: center; }
      .settings-body { padding: 0 14px 18px; }
      .input-with-button { align-items: stretch; }
      #wifiSetup .input-with-button button { min-width: 82px; padding-inline: 12px; }
      .wifi-dialog-body { padding: 16px; }
    }
    @media (prefers-reduced-motion: reduce) {
      *, *::before, *::after {
        animation: none !important;
        transition: none !important;
        scroll-behavior: auto !important;
      }
      .section-reveal { opacity: 1; transform: none; }
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
          <span>Mumins Personal Assistant</span>
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

  <!-- Wi-Fi Disconnected Notice -->
  <aside id="onlineOfflineNotice" class="online-notice" role="status" aria-live="polite" hidden>
    <span style="font-size:1.3rem">⚠️</span>
    <div>Prayer-Bot is not connected to a Wi-Fi network. Choose any compatible 2.4 GHz router or phone hotspot below. Online services also require that network to have internet access; offline Azan beep, Pomodoro, and Dhikr remain available.</div>
  </aside>

  <!-- Wi-Fi Setup Section -->
  <section id="wifiSetup" class="card wide" style="margin-bottom:24px">
    <h2><span>📶</span> Wi-Fi Connection Setup</h2>
    <p>Choose any compatible 2.4 GHz Wi-Fi network: a router, Android/iPhone hotspot, or another device's hotspot. Credentials are saved only after the connection and DHCP address are verified.</p>
    <div class="hint" style="margin-bottom:18px;line-height:1.6">
      <strong>Requirements:</strong> the ESP32 uses 2.4 GHz Wi-Fi (5 GHz-only networks are not supported). WPA2-Personal is recommended; compatibility with WPA3-only or enterprise networks depends on the network and firmware.<br>
      For a phone hotspot, enable its 2.4 GHz compatibility/band and WPA2 setting when available. On iPhone, turn on <strong>Settings → Personal Hotspot → Maximize Compatibility</strong> if available.<br>
      Join the <strong id="setupApNameLabel">Prayer-Bot</strong> setup hotspot (password: <strong>12345678</strong>). If the hotspot phone cannot stay connected to Prayer-Bot Wi-Fi while sharing, use a second phone or computer to open this page.
    </div>

    <div class="fields">
      <div class="field">
        <label><span class="wifi-step-badge">1</span> Nearby Wi-Fi networks</label>
        <div class="input-with-button">
          <button id="scanWifi" class="primary" type="button" onclick="scanWifiNetworks()">🔍 Scan nearby networks</button>
        </div>
        <small class="hint">Scan and select any nearby Wi-Fi network or hotspot, or enter its SSID manually.</small>
      </div>
      <div class="field">
        <label for="wifiSsid"><span class="wifi-step-badge">2</span> Wi-Fi network name (SSID)</label>
        <input id="wifiSsid" maxlength="32" autocomplete="username" placeholder="Enter Wi-Fi or hotspot name" required>
      </div>
      <div class="field" style="grid-column:span 2">
        <label for="wifiPass"><span class="wifi-step-badge">3</span> Wi-Fi password</label>
        <div class="input-with-button">
          <input id="wifiPass" type="password" maxlength="63" autocomplete="off" placeholder="Enter Wi-Fi password (blank if open)">
          <button type="button" onclick="togglePasswordVisibility('wifiPass', this)" style="flex:0 0 auto" aria-label="Toggle password view">👁 Show</button>
        </div>
        <small class="hint">Enter the network password. Leave blank only for an open network; WPA2-Personal is recommended.</small>
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
          <div class="hero-stat"><span>Home Wi-Fi IP</span><strong><a id="homeIpLink" href="#">—</a></strong></div>
        </div>
      </div>
    </article>

    <!-- Quick Device Stats Overview -->
    <article class="card wide">
      <h2><span>📊</span> Device &amp; Connection Status</h2>
      <div class="stats">
        <div class="stat"><span>Connection</span><strong id="network">Loading…</strong></div>
        <div class="stat"><span>Current Dashboard IP (HTTP)</span><strong><a id="ipLink" href="#"><span id="ip">—</span></a></strong></div>
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
          <span class="prayer-time" id="fajr">4:41 AM</span>
        </div>
        <div class="prayer-card" id="card-sunrise">
          <span class="prayer-icon">🌅</span>
          <span class="prayer-name">Sunrise</span>
          <span class="prayer-arabic">الشروق</span>
          <span class="prayer-time" id="sunrise">5:57 AM</span>
        </div>
        <div class="prayer-card" id="card-dhuhr">
          <span class="prayer-icon">☀️</span>
          <span class="prayer-name">Dhuhr</span>
          <span class="prayer-arabic">الظهر</span>
          <span class="prayer-time" id="dhuhr">11:44 AM</span>
        </div>
        <div class="prayer-card" id="card-asr">
          <span class="prayer-icon">🌤</span>
          <span class="prayer-name">Asr</span>
          <span class="prayer-arabic">العصر</span>
          <span class="prayer-time" id="asr">3:53 PM</span>
        </div>
        <div class="prayer-card" id="card-maghrib">
          <span class="prayer-icon">🌇</span>
          <span class="prayer-name">Maghrib</span>
          <span class="prayer-arabic">المغرب</span>
          <span class="prayer-time" id="maghrib">5:30 PM</span>
        </div>
        <div class="prayer-card" id="card-isha">
          <span class="prayer-icon">🌙</span>
          <span class="prayer-name">Isha</span>
          <span class="prayer-arabic">العشاء</span>
          <span class="prayer-time" id="isha">6:41 PM</span>
        </div>
      </div>
      <div class="fasting-times" aria-label="Sahari and Iftar times">
        <div class="fasting-time">
          <span>🌙 Sahari ends (Fajr)</span>
          <strong id="sahariEnd">4:41 AM</strong>
        </div>
        <div class="fasting-time">
          <span>🌇 Iftar (Maghrib)</span>
          <strong id="iftarTime">5:30 PM</strong>
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
          <span class="prohibited-range" id="afterSunriseRange">5:57 AM – 6:12 AM</span>
          <span class="prohibited-state" id="afterSunriseState">Not active now</span>
          <p class="hint">From sunrise until the sun rises a spear's height (~15 min).</p>
        </div>
        <div class="prohibited-card" id="card-prohibited-zawal">
          <div class="prohibited-title"><span>☀️</span> 2. At Zawal (استواء الشمس)</div>
          <span class="prohibited-range" id="atZawalRange">11:34 AM – 11:44 AM</span>
          <span class="prohibited-state" id="atZawalState">Not active now</span>
          <p class="hint">Exact solar noon just before Dhuhr azan starts (~10 min).</p>
        </div>
        <div class="prohibited-card" id="card-prohibited-sunset">
          <div class="prohibited-title"><span>🌇</span> 3. Before Sunset (غروب الشمس)</div>
          <span class="prohibited-range" id="beforeSunsetRange">5:15 PM – 5:30 PM</span>
          <span class="prohibited-state" id="beforeSunsetState">Not active now</span>
          <p class="hint">When the sun pales until it sets into Maghrib (~15 min).</p>
        </div>
      </div>
      <p id="prohibitedNote" class="hint" style="margin-top:14px">Calculated directly from today's Sunrise, Dhuhr, and Maghrib times using the configured offsets.</p>
    </article>

    <!-- Al-Quran Audio Player -->
    <article class="card" id="quran-player">
      <h2><span>✨</span> Quran Assistant</h2>
      <p class="hint">Find a Surah by voice or text, then listen on this phone or Prayer-Bot. Common Surah names and numbers are matched locally; flexible requests use Gemini 3.8 Flash through the device, so the API key is not exposed to the browser.</p>

      <div class="assistant-panel">
        <div id="assistantVoiceHeading" class="assistant-heading" data-voice-phase="idle">
          <div id="voiceOrb" class="voice-orb" aria-hidden="true">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="2" width="6" height="12" rx="3"></rect><path d="M5 10v2a7 7 0 0 0 14 0v-2M12 19v3m-4 0h8"></path></svg>
          </div>
          <div>
            <div class="assistant-greeting">Choose English or Bangla, then speak. Secure pages stream voice to Gemini Live; browser speech recognition remains available as a fallback.</div>
            <div class="voice-bars" aria-hidden="true"><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i></div>
            <div id="voiceActivity" class="voice-activity" data-phase="connecting" role="status" aria-live="polite" hidden>
              <svg class="voice-timer-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" aria-hidden="true"><circle cx="12" cy="13" r="8"></circle><path d="M12 9v4l2.5 1.5M9 2h6m-3 0v3"></path></svg>
              <span id="voiceActivityPhase">Connecting</span>
              <strong id="voiceElapsed">0.0 s</strong>
            </div>
          </div>
        </div>
        <div id="browserVoiceStatus" class="assistant-result" role="status" aria-live="polite">Ready. Ask for a Surah or the Prayer-Bot Azan alarm.</div>
        <div class="actions">
          <select id="voiceLanguage" data-requires-wifi aria-label="Voice language" style="max-width:150px;min-height:44px"><option value="en-US">English</option><option value="bn-BD">বাংলা (Bangla)</option></select>
          <button id="browserVoiceButton" type="button" data-requires-wifi style="min-height:44px">🎙 Start voice input</button>
          <button id="browserVoiceCancel" type="button" onclick="cancelBrowserVoice()" style="min-height:44px" hidden>Cancel</button>
        </div>
        <div class="assistant-search">
          <input id="assistantQuery" type="search" maxlength="120" placeholder="Try “Play Surah Al-Kahf”" aria-label="Ask the Quran assistant">
          <button id="assistantAskButton" type="button" data-requires-wifi onclick="submitAssistantQuery()">Ask</button>
        </div>
        <div class="assistant-examples">Try: “Play Surah Ar-Rahman”, “Find Ya-Sin”, or “Play the Azan alarm”. The Azan action plays the device's notification tone, not a recorded call to prayer.</div>
      </div>

      <div class="field" style="margin-top:16px">
        <label for="quranOutput">Audio Playback Destination</label>
        <select id="quranOutput" data-requires-wifi>
          <option value="device" selected>🔊 Prayer-Bot Device Speaker (default)</option>
          <option value="phone">📱 This Phone / Browser (needs internet)</option>
        </select>
      </div>

      <div class="field">
        <label for="quranSurahSelect">Select Surah (1 to 114)</label>
        <select id="quranSurahSelect" data-requires-wifi>
          <option value="">Choose a Surah</option>
        </select>
      </div>
      <div class="actions quran-actions" style="margin-top:12px">
        <button type="button" class="primary quran-play" data-requires-wifi onclick="playQuran()">▶ Play Recitation</button>
        <button type="button" onclick="stopQuran()">⏹ Stop</button>
        <button type="button" data-requires-wifi onclick="prevQuran()">⏮ Prev</button>
        <button type="button" data-requires-wifi onclick="nextQuran()">⏭ Next</button>
      </div>

      <audio id="phoneQuranAudio" controls playsinline preload="none" data-requires-wifi style="display:none;width:100%;margin-top:14px"></audio>

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
            <option value="3">4. Qibla Direction</option>
            <option value="4">5. Weather &amp; Air Quality</option>
            <option value="5">6. Pomodoro Focus Timer</option>
            <option value="6">7. Dhikr / Zikir Counter</option>
            <option value="7">8. Al-Quran Surah Recitation</option>
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
      <div class="chart-scale-note" aria-hidden="true"><span>Minutes</span><span>Sessions</span></div>
      <div class="chart-wrap"><canvas id="weeklyChart" aria-label="Daily focus chart for the past week"></canvas></div>
      <div id="weeklyBreakdown" class="weekly-breakdown" aria-label="Daily Pomodoro totals for the past seven days"></div>
      <p id="statsNote" class="hint">Stored in LittleFS flash memory. On the device, long-touch the sensor for 3 seconds to start.</p>
    </article>

    <!-- Weather & Air Quality Card -->
    <article class="card wide" id="weather">
      <h2><span>⛅</span> Local Weather &amp; Air Quality</h2>
      <div class="stats">
        <div class="stat"><span>City</span><strong id="weatherCity">—</strong></div>
        <div class="stat"><span>Condition</span><strong id="weatherCondition">—</strong></div>
        <div class="stat"><span>Air temperature</span><strong id="temperature">—</strong></div>
        <div class="stat"><span>Relative Humidity</span><strong id="humidity">—</strong></div>
        <div class="stat"><span>OpenWeather AQI (1–5)</span><strong id="aqi">—</strong></div>
        <div class="stat"><span>PM2.5</span><strong id="pm25">—</strong></div>
        <div class="stat"><span>PM10</span><strong id="pm10">—</strong></div>
        <div class="stat"><span>Ozone (O₃)</span><strong id="ozone">—</strong></div>
        <div class="stat"><span>Nitrogen dioxide (NO₂)</span><strong id="no2">—</strong></div>
        <div class="stat"><span>Carbon monoxide (CO)</span><strong id="co">—</strong></div>
        <div class="stat"><span>Sulphur dioxide (SO₂)</span><strong id="so2">—</strong></div>
        <div class="stat"><span>Nitrogen monoxide (NO)</span><strong id="no">—</strong></div>
        <div class="stat"><span>Ammonia (NH₃)</span><strong id="nh3">—</strong></div>
      </div>
      <p class="hint" style="margin-top:12px">Current air temperature and pollution readings come from OpenWeatherMap at the configured coordinates. Phone weather apps may show a different value because they can use another provider, nearby measurement point, update time, or a feels-like temperature. OpenWeatherMap's AQI uses its own 1–5 scale; it is not the US AQI scale. Air-quality data requires an active API key.</p>
    </article>

    <!-- Device Settings Accordion -->
    <details class="card wide settings" id="settings">
      <summary>
        <span><span>⚙</span> Device Preferences &amp; Coordinates</span>
        <span class="badge">Location &amp; Calculation</span>
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
          <div class="field" style="grid-column:span 2">
            <button id="useDeviceLocation" type="button" class="secondary" onclick="useDeviceLocation()">📍 Use device location</button>
            <p class="hint">Requires browser location permission and a secure page. The device serves HTTP; enter coordinates manually or use a secure proxy.</p>
          </div>
          <div class="field"><label for="city">City Name</label><input id="city" maxlength="63" required></div>
          <div class="field"><label for="timezone">Timezone Label</label><input id="timezone" maxlength="39" placeholder="Asia/Dhaka" required></div>
          <div class="field"><label for="lat">Latitude (-90 to 90)</label><input id="lat" type="number" min="-90" max="90" step="any" required></div>
          <div class="field"><label for="lon">Longitude (-180 to 180)</label><input id="lon" type="number" min="-180" max="180" step="any" required></div>
          <div class="field"><label for="utcOffset">UTC Offset (hours, e.g. +6)</label><input id="utcOffset" type="number" min="-12" max="14" step="0.25" required></div>
          <div class="field"><label for="hijriOffset">Hijri Day Offset (-2 to +2)</label><input id="hijriOffset" type="number" min="-2" max="2" step="1"></div>
          <p class="hint" style="grid-column:span 2">The Qibla direction uses the saved latitude and longitude. Choose a city preset or update the coordinates for your location, then select Save &amp; Apply Settings; changing only the city-name label does not change the direction. For best compass accuracy, keep the device level and away from speakers, magnets, and metal while slowly turning it through a full 360° to calibrate. The compass uses magnetic heading without local declination correction, so the displayed direction is approximate.</p>
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

        <div class="actions" style="margin-top:20px">
          <button class="primary" type="button" onclick="saveSettings()">💾 Save &amp; Apply Settings</button>
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

<dialog id="wifiScanDialog" class="wifi-scan-dialog" aria-labelledby="wifiScanTitle">
  <div class="wifi-dialog-body">
    <div class="wifi-dialog-header">
      <div>
        <h2 id="wifiScanTitle">Nearby Wi-Fi networks</h2>
        <p class="hint">Scanning the device's 2.4 GHz Wi-Fi range.</p>
      </div>
      <button type="button" onclick="closeWifiScan()" aria-label="Close Wi-Fi scan">Close</button>
    </div>
    <div id="wifiScanStatus" class="wifi-scan-status" role="status" aria-live="polite" data-loading="true">
      <span class="wifi-scan-spinner" aria-hidden="true"></span>
      <span>Scanning nearby networks…</span>
    </div>
    <div id="wifiScanList" class="wifi-scan-list"></div>
  </div>
</dialog>

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

  function showDashboard() {
    dashboardReady = true;
    byId('wifiSetup').hidden = true;
    byId('dashboardContent').hidden = false;
    byId('savebar').hidden = false;
  }

  function openWifiSetup() {
    byId('wifiSetup').hidden = false;
    byId('continueWifi').hidden = true;
    byId('wifiMessage').textContent = 'Ready to connect to any compatible 2.4 GHz Wi-Fi network.';
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

  function closeWifiScan() {
    const dialog = byId('wifiScanDialog');
    if (typeof dialog.close === 'function') dialog.close();
    else dialog.removeAttribute('open');
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

  function setOnlineAvailability(connected, connecting = false) {
    routerWifiConnected = Boolean(connected);
    const notice = byId('onlineOfflineNotice');
    notice.hidden = routerWifiConnected;
    if (!routerWifiConnected) {
      notice.lastElementChild.textContent = connecting
        ? 'Prayer-Bot is trying to connect. Online services will be available after the connection succeeds. Use a compatible 2.4 GHz router or phone hotspot; WPA2-Personal is recommended.'
        : 'Prayer-Bot is not connected to a Wi-Fi network. Choose any compatible 2.4 GHz router or phone hotspot below. Online services also require that network to have internet access; offline Azan beep, Pomodoro, and Dhikr remain available.';
    }
    
    const badge = byId('headerStatusBadge');
    const netShort = byId('networkShort');
    if (routerWifiConnected) {
      badge.className = 'badge badge-online';
      netShort.textContent = 'Wi-Fi';
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
      phonePlaybackStatus = '';
      put('quranStreamStatus', 'Stopped: Wi-Fi is disconnected.');
    }
  }

  async function ensureRouterConnection() {
    const status = await api('/api/status');
    setOnlineAvailability(status.network?.connected, status.network?.connecting);
    if (!routerWifiConnected) throw new Error('Not connected to Wi-Fi. Connect to a network with internet and try again.');
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
  }

  async function loadStatus() {
    const s = await api('/api/status');
    setOnlineAvailability(s.network?.connected, s.network?.connecting);
    put('network', s.network.connected ? 'Connected to Wi-Fi' : (s.network.connecting ? 'Connecting to Wi-Fi' : 'Device Hotspot Mode'));
    put('ip', s.network.ip);
    const currentIp = s.network.ip || '';
    byId('ipLink').href = currentIp ? `http://${currentIp}/` : '#';
    const homeIp = s.network.homeIp || '';
    const homeIpLink = byId('homeIpLink');
    homeIpLink.textContent = homeIp || 'Not connected';
    homeIpLink.href = homeIp ? `http://${homeIp}/` : '#';
    homeIpLink.setAttribute('aria-disabled', String(!homeIp));

    if (!dashboardReady && wifiSubmitted) {
      if (s.network.connectionFailed) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = 'Could not connect to that network. Check its name/password and 2.4 GHz or WPA2 compatibility, or try another router or phone hotspot.';
        byId('wifiMessage').classList.add('error');
        byId('continueWifi').hidden = true;
        byId('connectWifi').disabled = false;
      } else if (s.network.restarting) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = `Connected! Device is restarting. Assigned IP: ${s.network.homeIp || 'check your Wi-Fi network'}.`;
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden = true;
        byId('connectWifi').disabled = true;
        byId('wifiPass').value = '';
        byId('wifiPass').type = 'password';
      } else if (s.network.connected && !s.network.connecting) {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = 'Wi-Fi connected! Select "Next" to open the dashboard.';
        byId('wifiMessage').classList.remove('error');
        byId('continueWifi').hidden = false;
        byId('connectWifi').disabled = false;
        byId('wifiPass').value = '';
        byId('wifiPass').type = 'password';
      } else if (s.network.connecting) {
        byId('wifiMessage').textContent = 'Connecting to Wi-Fi…';
        byId('wifiMessage').classList.remove('error');
      } else {
        wifiSubmitted = false;
        byId('wifiMessage').textContent = s.network.authFailed
          ? 'Wi-Fi authentication failed. Check password.'
          : 'Could not connect. Check the network name/password and its Wi-Fi band/security compatibility.';
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
    put('sahariEnd', s.prayers.fajr && s.prayers.fajr !== '00:00' ? s.prayers.fajr : '—');
    put('iftarTime', s.prayers.maghrib && s.prayers.maghrib !== '00:00' ? s.prayers.maghrib : '—');
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
      const phoneSelected = byId('quranOutput').value === 'phone' &&
        phoneAudio && (Boolean(phoneAudio.currentSrc) || Boolean(phonePlaybackStatus));
      const phonePlaying = phoneSelected && !phoneAudio.paused;
      put('quranStreamStatus', phonePlaying
        ? 'Playing on this phone'
        : (phoneSelected
          ? (phonePlaybackStatus || 'Ready on this phone')
          : (s.quran.playing ? 'Playing on Prayer-Bot' : (s.quran.status || 'Idle'))));
      const qSel = byId('quranSurahSelect');
      if (qSel && s.quran.surahNumber && document.activeElement !== qSel) {
        qSel.value = s.quran.surahNumber;
      }
    }

    // Weather status
    put('weatherCity', s.weather.city || 'Dhaka');
    put('weatherCondition', s.weather.valid ? s.weather.condition : '—');
    put('temperature', s.weather.valid ? `${s.weather.temperatureC} °C` : 'Unavailable');
    put('humidity', s.weather.valid ? `${s.weather.humidity}%` : '—');
    put('aqi', s.weather.airQualityValid
      ? `${s.weather.aqi} · ${['', 'Good', 'Fair', 'Moderate', 'Poor', 'Very poor'][s.weather.aqi] || 'Unknown'}`
      : 'Unavailable');
    for (const key of ['pm25', 'pm10', 'ozone', 'no2', 'co', 'so2', 'no', 'nh3']) {
      put(key, s.weather.airQualityValid ? `${Number(s.weather[key]).toFixed(1)} µg/m³` : '—');
    }

    if (initialStatus) {
      byId('screenSelect').value = s.screen;
      initialStatus = false;
    }
    return Boolean(s.prayers.liveSynced);
  }

  function formatPrayerTime(value) {
    const match = /^(\d{1,2}):(\d{2})/.exec(String(value));
    if (!match) return value;
    const hour = Number(match[1]);
    const minute = Number(match[2]);
    const hour12 = hour % 12 || 12;
    return `${hour12}:${String(minute).padStart(2, '0')} ${hour < 12 ? 'AM' : 'PM'}`;
  }

  async function triggerPrayerSync() {
    const btn = byId('syncPrayerBtn');
    if (btn) btn.disabled = true;
    message('Syncing prayer & prohibited schedule with Aladhan API…');
    try {
      const res = await api('/api/prayer/refresh', { method: 'POST' });
      if (!res.refreshSucceeded) {
        throw new Error('The device could not confirm a live prayer-time refresh.');
      }
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
          put('fajr', formatPrayerTime(t.Fajr)); put('sunrise', formatPrayerTime(t.Sunrise)); put('dhuhr', formatPrayerTime(t.Dhuhr));
          put('asr', formatPrayerTime(t.Asr)); put('maghrib', formatPrayerTime(t.Maghrib)); put('isha', formatPrayerTime(t.Isha));
          put('sahariEnd', formatPrayerTime(t.Fajr)); put('iftarTime', formatPrayerTime(t.Maghrib));
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

  async function useDeviceLocation() {
    if (!window.isSecureContext) {
      message('Device location requires a secure browser page. Enter coordinates manually or use a secure HTTPS proxy.', true);
      return;
    }
    if (!navigator.geolocation) {
      message('This browser does not support device location. Choose a preset or enter coordinates manually.', true);
      return;
    }

    const button = byId('useDeviceLocation');
    button.disabled = true;
    button.textContent = 'Finding location…';
    try {
      const position = await new Promise((resolve, reject) => {
        navigator.geolocation.getCurrentPosition(resolve, reject, {
          enableHighAccuracy: false,
          maximumAge: 60000,
          timeout: 15000
        });
      });
      const { latitude, longitude, accuracy } = position.coords;
      if (!Number.isFinite(latitude) || !Number.isFinite(longitude) ||
          latitude < -90 || latitude > 90 ||
          longitude < -180 || longitude > 180) {
        throw new Error('The browser returned invalid coordinates.');
      }

      byId('city').value = 'Current location';
      byId('lat').value = latitude.toFixed(6);
      byId('lon').value = longitude.toFixed(6);
      const timeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
      if (timeZone) byId('timezone').value = timeZone;
      byId('utcOffset').value = (-new Date().getTimezoneOffset() / 60).toFixed(2);
      byId('locationPreset').value = 'custom';
      const accuracyText = Number.isFinite(accuracy)
        ? ` Approximate accuracy: ±${Math.round(accuracy)} m.`
        : '';
      message(`Coordinates filled from your device.${accuracyText} Verify the city/area label and select Save & Apply Settings.`);
    } catch (error) {
      const messages = {
        1: 'Location permission was denied. Allow it in your browser settings or enter coordinates manually.',
        2: 'Your device could not determine a location. Try again or choose a preset.',
        3: 'Location lookup timed out. Try again or choose a preset.'
      };
      message(messages[error.code] || error.message || 'Could not get device location.', true);
    } finally {
      button.disabled = false;
      button.textContent = '📍 Use device location';
    }
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
    const w = rect.width, h = rect.height, pad = { l: 38, r: 34, t: 16, b: 32 };
    ctx.clearRect(0, 0, w, h);
    const maxMinutes = Math.max(60, ...days.map(d => Number(d.focusMinutes) || 0));
    const maxSessions = Math.max(1, ...days.map(d => Number(d.started) || 0));
    const minuteScale = Math.ceil(maxMinutes / 30) * 30;
    const sessionScale = Math.max(4, Math.ceil(maxSessions / 2) * 2);
    const chartW = w - pad.l - pad.r, chartH = h - pad.t - pad.b, step = chartW / days.length;
    ctx.font = '12px system-ui';
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    for (let tick = 0; tick <= 4; tick++) {
      const progress = tick / 4;
      const y = pad.t + chartH * (1 - progress);
      const minutes = Math.round(minuteScale * progress);
      const sessions = Math.round(sessionScale * progress);
      ctx.strokeStyle = '#e2e8f0';
      ctx.beginPath();
      ctx.moveTo(pad.l, y);
      ctx.lineTo(w - pad.r, y);
      ctx.stroke();
      ctx.fillStyle = '#64748b';
      ctx.fillText(String(minutes), pad.l - 6, y);
      ctx.textAlign = 'left';
      ctx.fillText(String(sessions), w - pad.r + 6, y);
      ctx.textAlign = 'right';
    }
    ctx.textAlign = 'center';
    ctx.textBaseline = 'top';
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const focusMinutes = Number(day.focusMinutes) || 0;
      const barH = chartH * focusMinutes / minuteScale;
      const gradient = ctx.createLinearGradient(0, pad.t + chartH - barH, 0, pad.t + chartH);
      gradient.addColorStop(0, '#21a486');
      gradient.addColorStop(1, '#08705f');
      ctx.fillStyle = gradient;
      const barWidth = Math.min(24, step * 0.48);
      if (barH > 0) {
        ctx.fillRect(x - barWidth / 2, pad.t + chartH - barH, barWidth, barH);
      }
      ctx.fillStyle = '#475569';
      ctx.fillText(String(day.weekday || ''), x, h - pad.b + 8);
    });
    ctx.strokeStyle = '#d97706';
    ctx.lineWidth = 2;
    ctx.beginPath();
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const y = pad.t + chartH - chartH * (Number(day.started) || 0) / sessionScale;
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    });
    ctx.stroke();
    days.forEach((day, i) => {
      const x = pad.l + step * (i + 0.5);
      const y = pad.t + chartH - chartH * (Number(day.started) || 0) / sessionScale;
      ctx.fillStyle = '#fff';
      ctx.beginPath();
      ctx.arc(x, y, 4, 0, Math.PI * 2);
      ctx.fill();
      ctx.strokeStyle = '#d97706';
      ctx.lineWidth = 2;
      ctx.stroke();
    });
    canvas.setAttribute('aria-label',
      `Weekly Pomodoro chart. Teal bars show focus minutes up to ${minuteScale} minutes; amber line shows sessions started up to ${sessionScale}.`);

    const breakdown = byId('weeklyBreakdown');
    breakdown.replaceChildren();
    days.forEach((day, index) => {
      const item = document.createElement('div');
      item.className = `weekly-day${index === days.length - 1 ? ' today' : ''}`;
      const weekday = document.createElement('strong');
      weekday.textContent = String(day.weekday || 'Day');
      const focus = document.createElement('span');
      focus.textContent = `${Number(day.focusMinutes) || 0} min focus`;
      const started = document.createElement('span');
      started.textContent = `${Number(day.started) || 0} started`;
      const completed = document.createElement('span');
      completed.textContent = `${Number(day.completed) || 0} done`;
      item.append(weekday, focus, started, completed);
      breakdown.append(item);
    });
  }

  let weeklyStatsCache = null;
  let weeklyChartResizeTimer = 0;
  async function loadWeeklyStats() {
    try {
      const stats = await api('/api/stats');
      if (!Array.isArray(stats.week) || stats.week.length !== 7) throw new Error('Invalid statistics');
      weeklyStatsCache = stats;
      put('weeklyStarted', stats.totalStarted);
      put('weeklyCompleted', stats.totalCompleted);
      put('weeklyFocus', `${stats.totalFocusMinutes} min`);
      drawWeeklyChart(stats.week);
    } catch(e) {
      put('statsNote', 'Weekly history is temporarily unavailable. Check the device connection and refresh.');
    }
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
      await loadSettings();
      await loadStatus();
      message('Settings saved to device storage and applied.');
    } catch(e) {
      message(e.message, true);
    } finally {
      buttons.forEach(b => { b.disabled = false; b.textContent = '💾 Save & Apply Settings'; });
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
    sel.add(new Option('Choose a Surah', ''));
    surahs.forEach((name, i) => {
      sel.add(new Option(`${i + 1}. ${name}`, i + 1));
    });
    sel.value = '';
  }

  let browserRecognition = null;
  let voiceAttemptGeneration = 0;
  let voiceCapturing = false;
  let voiceProcessingSpokenResult = false;
  let voiceCapturedTranscript = '';
  let voiceCaptureError = '';
  let voicePermissionPending = false;
  let browserMediaRecorder = null;
  let browserAudioStream = null;
  let browserAudioChunks = [];
  let liveWebSocket = null;
  let liveAudioContext = null;
  let liveAudioSource = null;
  let liveAudioProcessor = null;
  let liveAudioMute = null;
  let liveSetupComplete = false;
  let liveSessionEnding = false;
  let liveTranscript = '';
  let liveCaptureTimer = null;
  let liveEndTimer = null;
  let liveTranscriptFinalizing = false;
  let liveSetupReject = null;
  let voiceTimerEnabled = false;
  let voiceTimerStartedAt = 0;
  let voiceTimerInterval = null;
  function updateVoiceActivityTimer() {
    if (!voiceTimerEnabled || !voiceTimerStartedAt) return;
    const elapsedSeconds = Math.max(0, performance.now() - voiceTimerStartedAt) / 1000;
    put('voiceElapsed', `${elapsedSeconds.toFixed(1)} s`);
  }
  function setBrowserVoiceState(listening, text, error = false) {
    const phase = !listening ? 'idle'
      : (voiceCapturing ? 'listening'
        : (voiceProcessingSpokenResult || liveSessionEnding ||
           /recogn|transcrib|finish|gemini|sending|heard|matched|found surah/i.test(text)
          ? 'recognizing' : 'connecting'));
    const orb = byId('voiceOrb');
    orb?.classList.toggle('listening', phase === 'listening');
    orb?.classList.toggle('connecting', phase === 'connecting');
    orb?.classList.toggle('recognizing', phase === 'recognizing');
    byId('assistantVoiceHeading')?.setAttribute('data-voice-phase', phase);
    byId('browserVoiceButton').disabled =
      (listening && !voiceCapturing) || !routerWifiConnected;
    byId('voiceLanguage').disabled = listening || !routerWifiConnected;
    byId('browserVoiceButton').textContent = voiceCapturing
      ? '⏹ Stop listening'
      : '🎙 Start voice input';
    byId('assistantAskButton').disabled = listening || !routerWifiConnected;
    byId('browserVoiceCancel').hidden = !listening;
    put('browserVoiceStatus', text);
    byId('browserVoiceStatus').classList.toggle('error', error);
    const activity = byId('voiceActivity');
    if (voiceTimerEnabled && listening) {
      activity.hidden = false;
      activity.dataset.phase = phase;
      put('voiceActivityPhase',
        phase === 'listening' ? 'Listening'
          : (phase === 'recognizing' ? 'Recognizing' : 'Connecting'));
      if (voiceTimerInterval === null) {
        updateVoiceActivityTimer();
        voiceTimerInterval = window.setInterval(updateVoiceActivityTimer, 100);
      }
    } else {
      activity.hidden = true;
      if (voiceTimerInterval !== null) {
        window.clearInterval(voiceTimerInterval);
        voiceTimerInterval = null;
      }
      if (!listening) {
        voiceTimerEnabled = false;
        voiceTimerStartedAt = 0;
      }
    }
  }

  function initializeSectionReveals() {
    const sections = document.querySelectorAll('#wifiSetup, #dashboardContent > .card');
    if (!('IntersectionObserver' in window)) {
      sections.forEach(section => section.classList.add('is-visible'));
      return;
    }

    const observer = new IntersectionObserver(entries => {
      entries.forEach(entry => {
        if (!entry.isIntersecting) return;
        entry.target.classList.add('is-visible');
        observer.unobserve(entry.target);
      });
    }, { threshold: 0.08, rootMargin: '0px 0px -20px 0px' });

    sections.forEach((section, index) => {
      section.classList.add('section-reveal');
      section.style.setProperty('--reveal-delay', `${Math.min(index * 35, 175)}ms`);
      observer.observe(section);
    });
  }

  function initializeSectionNavigation() {
    const links = [...document.querySelectorAll('.nav-bar .nav-pill[href^="#"]')];
    const targets = links
      .map(link => byId(link.hash.slice(1)))
      .filter(Boolean);
    if (!('IntersectionObserver' in window)) return;

    const observer = new IntersectionObserver(entries => {
      const current = entries
        .filter(entry => entry.isIntersecting)
        .sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top)[0];
      if (!current) return;

      links.forEach(link => {
        const active = link.hash === `#${current.target.id}`;
        link.classList.toggle('active', active);
        if (active) link.setAttribute('aria-current', 'location');
        else link.removeAttribute('aria-current');
      });
    }, { rootMargin: '-148px 0px -68% 0px', threshold: 0 });

    targets.forEach(target => observer.observe(target));
  }

  function initializeScrollHeader() {
    const header = document.querySelector('.app-bar');
    let updatePending = false;
    header.classList.toggle('is-scrolled', window.scrollY > 8);
    window.addEventListener('scroll', () => {
      if (updatePending) return;
      updatePending = true;
      window.requestAnimationFrame(() => {
        header.classList.toggle('is-scrolled', window.scrollY > 8);
        updatePending = false;
      });
    }, { passive: true });
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

  function findLocalAssistantCommand(transcript) {
    const surahNumber = findSpokenSurah(transcript);
    if (surahNumber) return { intent: 'surah', number: surahNumber };

    const spoken = normalizeSurahSpeech(transcript);
    const numberedSurah = spoken.match(/\b(?:surah|chapter)\s+(?:number\s+)?(\d{1,3})\b/);
    if (numberedSurah) {
      const number = Number(numberedSurah[1]);
      if (number >= 1 && number <= surahs.length)
        return { intent: 'surah', number };
    }

    const asksForAzan = /\b(?:play|start|trigger|turn on)\b.*\b(?:azan|adhan)\b|\b(?:azan|adhan)\s+(?:alarm|tone)\b/.test(spoken);
    if (asksForAzan) return { intent: 'azan' };
    return null;
  }

  let phonePlaybackGeneration = 0;
  let phonePlaybackStallTimer = null;
  let phonePlaybackStatus = '';
  function clearPhonePlaybackStallTimer() {
    if (phonePlaybackStallTimer !== null) {
      window.clearTimeout(phonePlaybackStallTimer);
      phonePlaybackStallTimer = null;
    }
  }

  function startPhoneQuranPlayback(audio, primaryUrl, fallbackUrl, generation) {
    let usingFallback = false;
    const setPhoneStatus = text => {
      if (generation !== phonePlaybackGeneration) return;
      phonePlaybackStatus = text;
      put('quranStreamStatus', text);
    };
    const clearStalledTimer = () => {
      clearPhonePlaybackStallTimer();
    };
    const startFallback = () => {
      if (generation !== phonePlaybackGeneration || usingFallback) {
        return Promise.resolve(false);
      }
      usingFallback = true;
      clearStalledTimer();
      setPhoneStatus('Primary stream unavailable. Trying alternate recitation source…');
      audio.src = fallbackUrl;
      audio.load();
      return Promise.resolve().then(() => audio.play()).then(() => true).catch(error => {
        if (generation === phonePlaybackGeneration) {
          setPhoneStatus(error.name === 'NotAllowedError'
            ? 'Tap play in the audio controls to allow this phone to start recitation.'
            : 'Alternate recitation source could not start. Check this phone’s internet connection.');
        }
        return false;
      });
    };

    audio.onerror = () => {
      if (usingFallback) {
        clearStalledTimer();
        setPhoneStatus('Recitation could not load. Check this phone’s internet connection and try again.');
      } else {
        void startFallback();
      }
    };
    audio.onplaying = () => {
      if (generation !== phonePlaybackGeneration) return;
      clearStalledTimer();
      setPhoneStatus('Playing recitation on this phone');
    };
    audio.oncanplay = () => clearStalledTimer();
    audio.onwaiting = audio.onstalled = () => {
      if (generation !== phonePlaybackGeneration ||
          phonePlaybackStallTimer !== null) return;
      setPhoneStatus(usingFallback
        ? 'Buffering alternate stream… check the phone’s internet connection.'
        : 'Buffering recitation…');
      phonePlaybackStallTimer = window.setTimeout(() => {
        phonePlaybackStallTimer = null;
        if (usingFallback) {
          setPhoneStatus(
            'Still buffering. Check that this phone has internet access, or select the Prayer-Bot speaker.');
        } else {
          void startFallback();
        }
      }, 12000);
    };

    audio.pause();
    audio.src = primaryUrl;
    audio.load();
    audio.style.display = 'block';
    setPhoneStatus('Starting recitation on this phone…');
    return Promise.resolve().then(() => audio.play()).then(() => {
      return { playing: !audio.paused, destination: 'phone' };
    }).catch(error => {
      if (generation !== phonePlaybackGeneration) {
        return { playing: false, destination: 'phone' };
      }
      if (!usingFallback && error.name !== 'NotAllowedError') {
        return startFallback().then(playing => ({ playing, destination: 'phone' }));
      }
      setPhoneStatus(error.name === 'NotAllowedError'
        ? 'Tap play in the audio controls to allow this phone to start recitation.'
        : 'Could not start recitation. Check this phone’s internet connection and try again.');
      return { playing: false, destination: 'phone' };
    });
  }

  async function chooseSurah(number) {
    if (!Number.isInteger(number) || number < 1 || number > surahs.length) {
      throw new Error('Could not identify a Surah. Try again or pick from the list.');
    }
    byId('quranSurahSelect').value = number;
    const name = surahs[number - 1];
    put('quranNowPlaying', `${number}. ${name}`);

    if (byId('quranOutput').value === 'phone') {
      const audio = byId('phoneQuranAudio');
      const primary = `https://cdn.islamic.network/quran/audio-surah/128/ar.alafasy/${number}.mp3`;
      const backup = `https://cdn.mp3quran.net/audio/maher-muaiqly/r1/${String(number).padStart(3, '0')}.mp3`;
      const generation = ++phonePlaybackGeneration;
      return startPhoneQuranPlayback(audio, primary, backup, generation);
    }

    await ensureRouterConnection();
    ++phonePlaybackGeneration;
    const phoneAudio = byId('phoneQuranAudio');
    phoneAudio.pause();
    phoneAudio.removeAttribute('src');
    phoneAudio.load();
    phoneAudio.style.display = 'none';
    phonePlaybackStatus = '';
    clearPhonePlaybackStallTimer();
    const response = await api('/api/quran', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ action: 'play', surah: number }) });
    put('quranStreamStatus', response.status === 'queued' ? 'Connecting to device speaker…' : 'Starting recitation…');
    await loadStatus();
    return { playing: false, queued: response.status === 'queued', destination: 'device' };
  }

  async function waitForGeminiRecognition(requestId) {
    if (!Number.isInteger(requestId) || requestId < 1) {
      throw new Error('Device did not return a valid Gemini recognition ID.');
    }
    for (let attempt = 0; attempt < 60; attempt++) {
      await new Promise(resolve => setTimeout(resolve, 250));
      const result = await api(`/api/quran/recognize?id=${encodeURIComponent(requestId)}`);
      if (result.status === 'done' && result.intent === 'azan') return { intent: 'azan' };
      if (result.status === 'done' && result.intent === 'surah' &&
          Number.isInteger(result.surahNumber) && result.surahNumber >= 1 &&
          result.surahNumber <= surahs.length) {
        return { intent: 'surah', number: result.surahNumber };
      }
      if (result.status === 'error') throw new Error(result.error || 'Gemini could not understand that request.');
      if (result.status === 'unknown') throw new Error('Recognition request expired.');
      if (attempt % 4 === 0) setBrowserVoiceState(true, 'Gemini 3.8 Flash is identifying your request…');
    }
    throw new Error('Gemini recognition timed out.');
  }

  async function recognizeTranscriptWithGemini(transcript) {
    const queued = await api('/api/quran/recognize', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ transcript })
    });
    return waitForGeminiRecognition(queued.requestId);
  }

  async function recognizeAudioWithGemini(blob) {
    const mimeType = (blob.type || '').split(';')[0].toLowerCase();
    if (!['audio/mp4', 'audio/webm', 'audio/wav', 'audio/aac'].includes(mimeType)) {
      throw new Error('This browser recorded an unsupported audio format. Try typing the Surah name instead.');
    }
    const language = byId('voiceLanguage').value;
    const queued = await api(
      `/api/quran/recognize-audio?lang=${encodeURIComponent(language)}`, {
      method: 'POST',
      headers: { 'Content-Type': mimeType },
      body: blob
    });
    return waitForGeminiRecognition(queued.requestId);
  }

  async function runAssistantCommand(command, attempt) {
    if (attempt !== voiceAttemptGeneration) return;
    if (command.intent === 'azan') {
      await api('/api/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ playAzan: true })
      });
      setBrowserVoiceState(false, 'Playing the Prayer-Bot Azan notification tone. Touch or shake to stop.');
      await loadStatus();
      return;
    }
    const number = command.number;
    if (!Number.isInteger(number) || number < 1 || number > surahs.length) {
      throw new Error('Gemini did not find a supported Surah. Try its name or number.');
    }
    setBrowserVoiceState(true, `Found Surah ${number}, ${surahs[number - 1]}. Starting recitation…`);
    const playback = await chooseSurah(number);
    if (attempt === voiceAttemptGeneration) {
      const result = `Found Surah ${number}, ${surahs[number - 1]}.`;
      if (playback?.destination === 'phone') {
        setBrowserVoiceState(false, playback.playing
          ? `${result} Playing on this phone.`
          : `${result} Tap play in the audio controls to start.`);
      } else if (playback?.queued) {
        setBrowserVoiceState(false, `${result} Playback queued on the Prayer-Bot speaker.`);
      } else {
        setBrowserVoiceState(false, `${result} Check the playback status below.`);
      }
    }
  }

  async function handleCapturedAudio(blob, attempt) {
    if (!blob || blob.size === 0) throw new Error('No audio was captured. Tap the microphone and try again.');
    if (blob.size > 256 * 1024) throw new Error('Recording is too large. Keep it under 6 seconds and try again.');
    setBrowserVoiceState(true, 'Sending your recording to Gemini 3.8 Flash…');
    const command = await recognizeAudioWithGemini(blob);
    await runAssistantCommand(command, attempt);
  }

  async function handleAssistantQuery(transcript, attempt) {
    if (!transcript.trim()) throw new Error('Please say or type a Surah name or an Azan request.');
    let command = findLocalAssistantCommand(transcript);
    if (command) {
      setBrowserVoiceState(true, 'Matched your request on Prayer-Bot…');
    } else {
      setBrowserVoiceState(true, 'Trying optional Gemini 3.8 Flash recognition…');
      try {
        command = await recognizeTranscriptWithGemini(transcript);
      } catch(error) {
        throw new Error(`${error.message} Try a Surah name/number or configure an optional Gemini key.`);
      }
    }
    await runAssistantCommand(command, attempt);
  }

  async function submitAssistantQuery() {
    if (voiceCapturing || voicePermissionPending) cancelBrowserVoice();
    const attempt = ++voiceAttemptGeneration;
    const transcript = byId('assistantQuery').value.trim();
    try {
      await ensureRouterConnection();
      await handleAssistantQuery(transcript, attempt);
    } catch(e) {
      if (attempt === voiceAttemptGeneration) setBrowserVoiceState(false, e.message, true);
    }
  }

  function startBrowserRecognition(attempt) {
    const Recognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!Recognition || attempt !== voiceAttemptGeneration) return;

    const recognition = new Recognition();
    browserRecognition = recognition;
    recognition.lang = byId('voiceLanguage').value;
    recognition.continuous = false;
    recognition.interimResults = false;
    recognition.onstart = () => {
      if (attempt === voiceAttemptGeneration) {
        voiceCapturing = true;
        setBrowserVoiceState(true, 'Listening… tap stop when you finish speaking.');
      }
    };
    recognition.onresult = event => {
      if (attempt !== voiceAttemptGeneration) return;
      voiceCapturedTranscript = event.results?.[0]?.[0]?.transcript?.trim() || '';
    };
    recognition.onerror = event => {
      if (attempt !== voiceAttemptGeneration) return;
      voiceCaptureError = event.error === 'not-allowed'
        ? 'Allow microphone access in Safari settings and try again.'
        : `Speech error (${event.error}).`;
    };
    recognition.onend = () => {
      if (attempt !== voiceAttemptGeneration) return;
      if (browserRecognition === recognition) browserRecognition = null;
      voiceCapturing = false;
      if (voiceCaptureError) {
        setBrowserVoiceState(false, voiceCaptureError, true);
        return;
      }
      processCapturedVoice(attempt);
    };
    try {
      recognition.start();
    } catch(e) {
      if (attempt === voiceAttemptGeneration) {
        voiceCapturing = false;
        setBrowserVoiceState(false, 'Could not start speech recognition.', true);
      }
    }
  }

  function processCapturedVoice(attempt) {
    if (attempt !== voiceAttemptGeneration || voiceProcessingSpokenResult) return;
    const transcript = voiceCapturedTranscript;
    voiceCapturedTranscript = '';
    if (!transcript) {
      setBrowserVoiceState(false, 'No speech detected. Tap the microphone and try again.', true);
      return;
    }
    voiceProcessingSpokenResult = true;
    byId('assistantQuery').value = transcript;
    setBrowserVoiceState(true, 'Recognizing your request…');
    ensureRouterConnection()
      .then(() => handleAssistantQuery(transcript, attempt))
      .catch(error => {
        if (attempt === voiceAttemptGeneration) {
          setBrowserVoiceState(false, error.message, true);
        }
      })
      .finally(() => {
        if (attempt === voiceAttemptGeneration) voiceProcessingSpokenResult = false;
      });
  }

  function releaseBrowserAudioStream() {
    if (browserAudioStream) {
      browserAudioStream.getTracks().forEach(track => track.stop());
      browserAudioStream = null;
    }
  }

  function stopLiveAudioInput() {
    if (liveCaptureTimer !== null) {
      window.clearTimeout(liveCaptureTimer);
      liveCaptureTimer = null;
    }
    if (liveAudioProcessor) {
      liveAudioProcessor.onaudioprocess = null;
      liveAudioProcessor.disconnect();
      liveAudioProcessor = null;
    }
    if (liveAudioSource) {
      liveAudioSource.disconnect();
      liveAudioSource = null;
    }
    if (liveAudioMute) {
      liveAudioMute.disconnect();
      liveAudioMute = null;
    }
    if (liveAudioContext) {
      const context = liveAudioContext;
      liveAudioContext = null;
      if (context.state !== 'closed') {
        void context.close().catch(error =>
          console.warn('Could not close the live audio context:', error.message));
      }
    }
    if (browserAudioStream) {
      releaseBrowserAudioStream();
    }
  }

  function closeGeminiLiveSession() {
    if (liveEndTimer !== null) {
      window.clearTimeout(liveEndTimer);
      liveEndTimer = null;
    }
    stopLiveAudioInput();
    if (liveSetupReject) {
      const rejectSetup = liveSetupReject;
      liveSetupReject = null;
      rejectSetup(new Error('Live voice session was cancelled.'));
    }
    const socket = liveWebSocket;
    liveWebSocket = null;
    liveSetupComplete = false;
    liveSessionEnding = false;
    if (socket) {
      socket.onopen = null;
      socket.onmessage = null;
      socket.onerror = null;
      socket.onclose = null;
      if (socket.readyState === WebSocket.CONNECTING ||
          socket.readyState === WebSocket.OPEN) socket.close();
    }
  }

  function sendLiveAudioChunk(samples, sampleRate, attempt) {
    if (attempt !== voiceAttemptGeneration || !liveSetupComplete ||
        !liveWebSocket || liveWebSocket.readyState !== WebSocket.OPEN) return;
    if (liveWebSocket.bufferedAmount > 128 * 1024) {
      handleLiveConnectionFailure(attempt,
        'Live voice connection is too slow. Check Wi-Fi and try again.');
      return;
    }

    const ratio = sampleRate / 16000;
    const pcmSampleCount = Math.floor(samples.length / ratio);
    if (pcmSampleCount < 1) return;
    const pcmBytes = new Uint8Array(pcmSampleCount * 2);
    const view = new DataView(pcmBytes.buffer);
    for (let i = 0; i < pcmSampleCount; i++) {
      const start = Math.floor(i * ratio);
      const end = Math.min(samples.length, Math.floor((i + 1) * ratio));
      let total = 0;
      for (let j = start; j < Math.max(start + 1, end); j++) total += samples[j];
      const sample = Math.max(-1, Math.min(1, total / Math.max(1, end - start)));
      view.setInt16(i * 2, sample < 0 ? sample * 32768 : sample * 32767, true);
    }

    let binary = '';
    for (let offset = 0; offset < pcmBytes.length; offset += 0x8000) {
      binary += String.fromCharCode(...pcmBytes.subarray(
        offset, Math.min(offset + 0x8000, pcmBytes.length)));
    }
    try {
      liveWebSocket.send(JSON.stringify({
        realtimeInput: {
          audio: {
            data: btoa(binary),
            mimeType: 'audio/pcm;rate=16000'
          }
        }
      }));
    } catch(error) {
      handleLiveConnectionFailure(attempt,
        'Could not stream microphone audio. Check Wi-Fi and try again.');
    }
  }

  function finishGeminiLiveTranscript(attempt) {
    if (attempt !== voiceAttemptGeneration || liveTranscriptFinalizing) return;
    liveTranscriptFinalizing = true;
    const transcript = liveTranscript.trim();
    closeGeminiLiveSession();
    voiceCapturing = false;
    if (!transcript) {
      liveTranscriptFinalizing = false;
      setBrowserVoiceState(false, 'No speech detected. Tap the microphone and try again.', true);
      return;
    }

    byId('assistantQuery').value = transcript;
    voiceProcessingSpokenResult = true;
    setBrowserVoiceState(true, `Heard: ${transcript}`);
    handleAssistantQuery(transcript, attempt)
      .catch(error => {
        if (attempt === voiceAttemptGeneration) setBrowserVoiceState(false, error.message, true);
      })
      .finally(() => {
        if (attempt === voiceAttemptGeneration) {
          voiceProcessingSpokenResult = false;
          liveTranscriptFinalizing = false;
        }
      });
  }

  function handleLiveServerMessage(message, attempt) {
    if (attempt !== voiceAttemptGeneration) return;
    if (message.error) {
      handleLiveConnectionFailure(attempt,
        message.error.message || 'Gemini Live could not process the voice request.');
      return;
    }
    if (message.setupComplete) {
      liveSetupComplete = true;
      voicePermissionPending = false;
      try {
        if (!liveAudioContext || !browserAudioStream) {
          throw new Error('This browser cannot stream microphone audio.');
        }
        liveAudioSource = liveAudioContext.createMediaStreamSource(browserAudioStream);
        liveAudioProcessor = liveAudioContext.createScriptProcessor(2048, 1, 1);
        liveAudioMute = liveAudioContext.createGain();
        liveAudioMute.gain.value = 0;
        const sampleRate = liveAudioContext.sampleRate;
        liveAudioProcessor.onaudioprocess = event => {
          sendLiveAudioChunk(
            event.inputBuffer.getChannelData(0),
            sampleRate,
            attempt);
        };
        liveAudioSource.connect(liveAudioProcessor);
        liveAudioProcessor.connect(liveAudioMute);
        liveAudioMute.connect(liveAudioContext.destination);
        voiceCapturing = true;
        byId('browserVoiceButton').classList.add('holding');
        setBrowserVoiceState(true,
          `Listening in ${byId('voiceLanguage').value === 'bn-BD' ? 'Bangla' : 'English'}… tap stop when finished.`);
        liveCaptureTimer = window.setTimeout(() => {
          if (attempt === voiceAttemptGeneration && voiceCapturing) stopVoiceCapture();
        }, 6000);
      } catch(error) {
        handleLiveConnectionFailure(attempt, error.message);
      }
      return;
    }

    const content = message.serverContent;
    if (!content) return;
    const interim = content.interimInputTranscription?.text || '';
    const finalized = content.inputTranscription?.text?.trim() || '';
    if (finalized) {
      liveTranscript = `${liveTranscript} ${finalized}`.trim();
      byId('assistantQuery').value = liveTranscript;
    } else if (interim) {
      byId('assistantQuery').value = `${liveTranscript} ${interim}`.trim();
    }
    if (liveSessionEnding && finalized) {
      finishGeminiLiveTranscript(attempt);
    }
  }

  function handleLiveConnectionFailure(attempt, reason) {
    if (attempt !== voiceAttemptGeneration) return;
    closeGeminiLiveSession();
    voicePermissionPending = false;
    voiceCapturing = false;
    byId('browserVoiceButton').classList.remove('holding');
    if (window.SpeechRecognition || window.webkitSpeechRecognition) {
      setBrowserVoiceState(true, 'Live voice unavailable; switching to browser speech recognition.');
      startBrowserRecognition(attempt);
    } else {
      setBrowserVoiceState(false, reason, true);
    }
  }

  async function beginGeminiLiveCapture(attempt) {
    voicePermissionPending = true;
    setBrowserVoiceState(true, 'Opening microphone for live voice input…');
    const AudioContextConstructor = window.AudioContext || window.webkitAudioContext;
    liveAudioContext = new AudioContextConstructor();
    await liveAudioContext.resume();
    browserAudioStream = await navigator.mediaDevices.getUserMedia({
      audio: { echoCancellation: true, noiseSuppression: true, autoGainControl: true }
    });
    if (attempt !== voiceAttemptGeneration) {
      releaseBrowserAudioStream();
      return;
    }
    setBrowserVoiceState(true, 'Starting secure live voice connection…');
    const tokenRequest = await api('/api/quran/live-token', { method: 'POST' });
    if (!Number.isInteger(tokenRequest.requestId) || tokenRequest.requestId < 1) {
      throw new Error('Prayer-Bot did not queue a Live API token request.');
    }
    let tokenReply = null;
    for (let attemptIndex = 0; attemptIndex < 60; attemptIndex++) {
      if (attempt !== voiceAttemptGeneration) return;
      await new Promise(resolve => window.setTimeout(resolve, 250));
      tokenReply = await api(
        `/api/quran/live-token?id=${encodeURIComponent(tokenRequest.requestId)}`);
      if (attempt !== voiceAttemptGeneration) return;
      if (tokenReply.status === 'done') break;
      if (tokenReply.status === 'error') {
        throw new Error(tokenReply.error || 'Gemini could not authorize live voice.');
      }
      if (tokenReply.status === 'unknown') {
        throw new Error('Prayer-Bot could not find the Live API token request.');
      }
      if (attemptIndex > 0 && attemptIndex % 8 === 0) {
        setBrowserVoiceState(true, 'Preparing secure Gemini Live access…');
      }
    }
    if (tokenReply?.status !== 'done') {
      throw new Error('Timed out while preparing secure Gemini Live access.');
    }
    if (attempt !== voiceAttemptGeneration) return;
    if (!tokenReply.token) throw new Error('Prayer-Bot did not return a Live API token.');

    const socketUrl =
      `wss://generativelanguage.googleapis.com/ws/google.ai.generativelanguage.v1beta.GenerativeService.BidiGenerateContentConstrained?access_token=${encodeURIComponent(tokenReply.token)}`;
    const socket = new WebSocket(socketUrl);
    liveWebSocket = socket;
    liveTranscript = '';
    liveTranscriptFinalizing = false;
    liveSessionEnding = false;
    liveSetupComplete = false;
    await new Promise((resolve, reject) => {
      liveSetupReject = reject;
      const setupTimeout = window.setTimeout(() => {
        if (!liveSetupComplete) {
          liveSetupReject = null;
          reject(new Error('Gemini Live connection timed out.'));
        }
      }, 12000);
      socket.onopen = () => {
        const setup = {
          setup: {
            model: 'models/gemini-3.5-transcribe-live',
            generationConfig: { responseModalities: ['TEXT'] },
            inputAudioTranscription: {
              languageCodes: [byId('voiceLanguage').value]
            }
          }
        };
        try {
          socket.send(JSON.stringify(setup));
        } catch(error) {
          window.clearTimeout(setupTimeout);
          reject(new Error('Could not configure the Gemini Live session.'));
        }
      };
      socket.onmessage = event => {
        let message;
        try {
          message = JSON.parse(event.data);
        } catch(error) {
          window.clearTimeout(setupTimeout);
          reject(new Error('Gemini Live returned an invalid response.'));
          return;
        }
        if (message.error && !liveSetupComplete) {
          window.clearTimeout(setupTimeout);
          liveSetupReject = null;
          reject(new Error(message.error.message ||
            'Gemini Live rejected the voice session.'));
          return;
        }
        if (message.setupComplete) {
          window.clearTimeout(setupTimeout);
          liveSetupReject = null;
          handleLiveServerMessage(message, attempt);
          resolve();
          return;
        }
        handleLiveServerMessage(message, attempt);
      };
      socket.onerror = () => {
        if (!liveSetupComplete) {
          window.clearTimeout(setupTimeout);
          liveSetupReject = null;
          reject(new Error('Could not connect to Gemini Live. Check Wi-Fi and API access.'));
        } else {
          handleLiveConnectionFailure(attempt,
            'Gemini Live disconnected. Check Wi-Fi and try again.');
        }
      };
      socket.onclose = () => {
        if (!liveSetupComplete) {
          window.clearTimeout(setupTimeout);
          liveSetupReject = null;
          reject(new Error('Gemini Live closed before voice setup completed.'));
        } else if (attempt === voiceAttemptGeneration && !liveSessionEnding) {
          handleLiveConnectionFailure(attempt,
            'Gemini Live disconnected. Check Wi-Fi and try again.');
        }
      };
    }).catch(error => {
      closeGeminiLiveSession();
      voicePermissionPending = false;
      throw error;
    });
  }

  async function beginVoiceCapture() {
    if (voiceCapturing || voicePermissionPending || voiceProcessingSpokenResult ||
        !routerWifiConnected) return;
    const attempt = ++voiceAttemptGeneration;
    voiceCaptureError = '';
    voiceCapturedTranscript = '';
    voiceTimerEnabled = true;
    voiceTimerStartedAt = performance.now();

    if (window.isSecureContext && window.WebSocket &&
        navigator.mediaDevices?.getUserMedia &&
        (window.AudioContext || window.webkitAudioContext)) {
      try {
        await beginGeminiLiveCapture(attempt);
        return;
      } catch(error) {
        if (attempt !== voiceAttemptGeneration) return;
        closeGeminiLiveSession();
        voicePermissionPending = false;
        voiceCapturing = false;
        console.warn('Gemini Live voice input unavailable:', error.message);
        if (!(window.SpeechRecognition || window.webkitSpeechRecognition)) {
          setBrowserVoiceState(false, error.message, true);
          return;
        }
      }
    }

    if (window.SpeechRecognition || window.webkitSpeechRecognition) {
      setBrowserVoiceState(true, 'Starting browser speech recognition…');
      startBrowserRecognition(attempt);
      return;
    }

    if (!window.isSecureContext) {
      setBrowserVoiceState(false, 'Direct recording requires a secure page. Use browser speech recognition or type your request, or connect through a secure HTTPS proxy.', true);
      return;
    }

    if (window.MediaRecorder && navigator.mediaDevices?.getUserMedia) {
      try {
        voicePermissionPending = true;
        setBrowserVoiceState(true, 'Waiting for microphone permission…');
        browserAudioStream = await navigator.mediaDevices.getUserMedia({
          audio: { echoCancellation: true, noiseSuppression: true, autoGainControl: true }
        });
        voicePermissionPending = false;
        if (attempt !== voiceAttemptGeneration) {
          releaseBrowserAudioStream();
          return;
        }
        const supportedTypes = ['audio/mp4', 'audio/webm;codecs=opus', 'audio/webm', 'audio/aac'];
        const mimeType = supportedTypes.find(type =>
          !MediaRecorder.isTypeSupported || MediaRecorder.isTypeSupported(type)
        );
        browserMediaRecorder = mimeType
          ? new MediaRecorder(browserAudioStream, { mimeType })
          : new MediaRecorder(browserAudioStream);
        browserAudioChunks = [];
        browserMediaRecorder.ondataavailable = event => {
          if (event.data && event.data.size > 0) browserAudioChunks.push(event.data);
        };
        browserMediaRecorder.onerror = () => {
          voiceCapturing = false;
          releaseBrowserAudioStream();
          setBrowserVoiceState(false, 'Audio recording failed. Check microphone permission and try again.', true);
        };
        const recorder = browserMediaRecorder;
        recorder.onstop = () => {
          const recordedType = (recorder.mimeType || browserAudioChunks[0]?.type || '')
            .split(';')[0].toLowerCase();
          const recording = new Blob(browserAudioChunks, { type: recordedType });
          browserAudioChunks = [];
          if (browserMediaRecorder === recorder) browserMediaRecorder = null;
          releaseBrowserAudioStream();
          if (attempt !== voiceAttemptGeneration) return;
          voiceCapturing = false;
          voiceProcessingSpokenResult = true;
          handleCapturedAudio(recording, attempt)
            .catch(error => {
              if (attempt === voiceAttemptGeneration) setBrowserVoiceState(false, error.message, true);
            })
            .finally(() => {
              if (attempt === voiceAttemptGeneration) voiceProcessingSpokenResult = false;
            });
        };
        browserMediaRecorder.start(250);
        voiceCapturing = true;
        byId('browserVoiceButton').classList.add('holding');
        setBrowserVoiceState(true, 'Recording… tap “Stop and send” when finished.');
        window.setTimeout(() => {
          if (attempt === voiceAttemptGeneration && browserMediaRecorder?.state === 'recording') {
            stopVoiceCapture();
          }
        }, 6000);
        return;
      } catch(error) {
        voicePermissionPending = false;
        releaseBrowserAudioStream();
        closeGeminiLiveSession();
        if (error.name === 'NotAllowedError' || error.name === 'SecurityError') {
          setBrowserVoiceState(false, 'Allow microphone access in Safari and open the dashboard through a secure HTTPS page.', true);
          return;
        }
      }
    }

    setBrowserVoiceState(false, 'Voice input is not supported by this browser. Type your request instead.', true);
  }

  function stopVoiceCapture() {
    byId('browserVoiceButton').classList.remove('holding');
    if (liveWebSocket && liveSetupComplete && voiceCapturing) {
      liveSessionEnding = true;
      voiceCapturing = false;
      stopLiveAudioInput();
      setBrowserVoiceState(true, 'Finishing live transcription…');
      try {
        liveWebSocket.send(JSON.stringify({
          realtimeInput: { audioStreamEnd: true }
        }));
      } catch(error) {
        handleLiveConnectionFailure(voiceAttemptGeneration,
          'Could not finish the live voice request. Try again.');
        return;
      }
      liveEndTimer = window.setTimeout(() => {
        if (liveSessionEnding) {
          if (liveTranscript.trim()) {
            finishGeminiLiveTranscript(voiceAttemptGeneration);
          } else {
            closeGeminiLiveSession();
            setBrowserVoiceState(false,
              'No speech detected. Tap the microphone and try again.', true);
          }
        }
      }, 5000);
      return;
    }
    if (browserMediaRecorder?.state === 'recording') {
      setBrowserVoiceState(true, 'Finishing recording…');
      browserMediaRecorder.stop();
      return;
    }
    if (browserRecognition) {
      setBrowserVoiceState(true, 'Finishing speech recognition…');
      try {
        browserRecognition.stop();
      } catch(e) {
        browserRecognition.abort();
      }
    }
  }

  function cancelBrowserVoice() {
    ++voiceAttemptGeneration;
    closeGeminiLiveSession();
    voicePermissionPending = false;
    voiceCapturing = false;
    voiceProcessingSpokenResult = false;
    liveTranscriptFinalizing = false;
    voiceCapturedTranscript = '';
    voiceCaptureError = '';
    byId('browserVoiceButton').classList.remove('holding');
    if (browserRecognition) browserRecognition.abort();
    browserRecognition = null;
    if (browserMediaRecorder?.state === 'recording') browserMediaRecorder.stop();
    browserMediaRecorder = null;
    browserAudioChunks = [];
    releaseBrowserAudioStream();
    setBrowserVoiceState(false, 'Voice input cancelled.');
  }
  const browserVoiceButton = byId('browserVoiceButton');
  browserVoiceButton.addEventListener('click', () => {
    if (voiceCapturing) stopVoiceCapture();
    else beginVoiceCapture();
  });
  byId('assistantQuery').addEventListener('keydown', event => {
    if (event.key === 'Enter') {
      event.preventDefault();
      submitAssistantQuery();
    }
  });

  async function playQuran() {
    const sel = byId('quranSurahSelect');
    const surah = Number(sel?.value);
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
      ++phonePlaybackGeneration;
      clearPhonePlaybackStallTimer();
      audio.pause();
      audio.onerror = null;
      audio.onplaying = null;
      audio.oncanplay = null;
      audio.onwaiting = null;
      audio.onstalled = null;
      audio.removeAttribute('src');
      audio.load();
      audio.style.display = 'none';
      phonePlaybackStatus = '';
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
      byId('wifiMessage').textContent = 'Connecting to Wi-Fi or hotspot… waiting for verification.';
      byId('wifiMessage').classList.remove('error');
    } catch(e) {
      byId('wifiMessage').textContent = e.message;
      byId('wifiMessage').classList.add('error');
      byId('connectWifi').disabled = false;
    }
  }

  async function scanWifiNetworks() {
    const button = byId('scanWifi');
    const dialog = byId('wifiScanDialog');
    const status = byId('wifiScanStatus');
    const statusText = status.lastElementChild;
    const list = byId('wifiScanList');
    const msg = byId('wifiMessage');
    const setScanStatus = (text, loading) => {
      statusText.textContent = text;
      status.dataset.loading = String(loading);
    };
    button.disabled = true;
    list.replaceChildren();
    setScanStatus('Scanning nearby networks…', true);
    if (typeof dialog.showModal === 'function') dialog.showModal();
    else dialog.setAttribute('open', '');
    msg.textContent = 'Scanning for nearby networks — please wait…';
    msg.classList.remove('error');
    try {
      await api('/api/wifi/scan', { method: 'POST' });
      let result;
      for (let attempt = 0; attempt < 20; attempt++) {
        await new Promise(r => setTimeout(r, 1000));
        result = await api('/api/wifi/scan');
        if (result.status === 'complete') break;
        const dots = '.'.repeat((attempt % 3) + 1);
        setScanStatus(`Scanning${dots}`, true);
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
      for (const n of sorted) {
        const choice = document.createElement('button');
        choice.type = 'button';
        choice.className = 'wifi-network-choice';
        const name = document.createElement('span');
        name.textContent = `${n.secured ? '🔒 ' : ''}${n.ssid}`;
        const signal = document.createElement('small');
        signal.textContent = `${n.rssi} dBm`;
        choice.append(name, signal);
        choice.addEventListener('click', () => {
          byId('wifiSsid').value = n.ssid;
          byId('wifiPass').value = '';
          byId('wifiPass').focus();
          msg.textContent = 'Network selected. Enter its password and tap Connect.';
          msg.classList.remove('error');
          closeWifiScan();
        });
        list.append(choice);
      }
      if (sorted.length) {
        setScanStatus(`Found ${sorted.length} network${sorted.length === 1 ? '' : 's'}. Choose a Wi-Fi network or hotspot.`, false);
        msg.textContent = `Found ${sorted.length} network${sorted.length === 1 ? '' : 's'}.`;
      } else {
        setScanStatus('No networks found. Enable the hotspot, move closer, and scan again.', false);
        msg.textContent = 'No networks found. Try scanning again.';
      }
    } catch(e) {
      setScanStatus(e.message, false);
      msg.textContent = `Wi-Fi scan failed: ${e.message}`;
      msg.classList.add('error');
    } finally {
      button.disabled = false;
    }
  }

  initializeSectionReveals();
  initializeSectionNavigation();
  initializeScrollHeader();
  initQuranDropdown();
  refreshAll();
  loadWeeklyStats();

  // Use one status poller; check provisioning attempts more frequently.
  let prayerReady = false;
  async function pollStatus() {
    try {
      prayerReady = await loadStatus();
    } catch(e) {
      if (wifiSubmitted) {
        byId('wifiMessage').textContent = 'Waiting for Prayer-Bot to reconnect…';
      }
    }
    setTimeout(pollStatus, wifiSubmitted ? 2000 : prayerReady ? 5000 : 3000);
  }
  setTimeout(pollStatus, 3000);
  setInterval(() => loadWeeklyStats(), 30000);
  window.addEventListener('resize', () => {
    clearTimeout(weeklyChartResizeTimer);
    weeklyChartResizeTimer = window.setTimeout(() => {
      if (weeklyStatsCache) drawWeeklyChart(weeklyStatsCache.week);
    }, 150);
  });
</script>
</body>
</html>
)rawliteral";

#endif
