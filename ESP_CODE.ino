#include <WiFi.h>
#include <WebServer.h>

// ==========================
// WIFI DETAILS
// ==========================

const char* ssid = "OP12";
const char* password = "12345678900";

// ==========================
// WEB SERVER
// ==========================

WebServer server(80);

// ==========================
// PIN DEFINITIONS
// ==========================

#define RELAY_PIN       23
#define PIR_PIN         4
#define TRIG_PIN        12
#define ECHO_PIN        14
#define CURRENT_SENSOR  34

// ==========================
// VARIABLES
// ==========================

bool humanDetected = false;
bool manualMode = false;
bool manualRelayState = false;

float current = 0;
float power = 0;
float distance = 0;

int pirState = 0;

// ==========================
// HUMAN DETECTED API
// ==========================

void handleHuman()
{
    humanDetected = true;
    Serial.println("HUMAN DETECTED FROM ESP32-CAM");
    server.send(200, "text/plain", "Human Detected");
}

// ==========================
// NO HUMAN API
// ==========================

void handleNoHuman()
{
    humanDetected = false;
    Serial.println("NO HUMAN FROM ESP32-CAM");
    server.send(200, "text/plain", "No Human");
}

// ==========================
// MANUAL RELAY ON
// ==========================

void handleRelayOn()
{
    manualMode = true;
    manualRelayState = true;
    digitalWrite(RELAY_PIN, LOW);
    server.send(200, "text/plain", "Relay ON");
}

// ==========================
// MANUAL RELAY OFF
// ==========================

void handleRelayOff()
{
    manualMode = true;
    manualRelayState = false;
    digitalWrite(RELAY_PIN, HIGH);
    server.send(200, "text/plain", "Relay OFF");
}

// ==========================
// AUTO MODE
// ==========================

void handleAutoMode()
{
    manualMode = false;
    server.send(200, "text/plain", "Auto Mode Enabled");
}

// ==========================
// WEB DASHBOARD
// ==========================

void handleRoot()
{
    bool relayOn = (digitalRead(RELAY_PIN) == LOW);

    String html = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>Smart Energy Monitor</title>
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Exo+2:wght@300;400;600;800&display=swap" rel="stylesheet">
<style>
  :root {
    --bg:        #080d14;
    --surface:   #0d1520;
    --border:    #1a2d45;
    --accent:    #00e5ff;
    --green:     #00ff88;
    --red:       #ff3b5c;
    --orange:    #ff9500;
    --yellow:    #ffe44d;
    --text:      #cde4f5;
    --muted:     #4a6a85;
    --glow:      0 0 12px rgba(0,229,255,0.35);
    --glow-g:    0 0 12px rgba(0,255,136,0.4);
    --glow-r:    0 0 12px rgba(255,59,92,0.4);
    --font-mono: 'Share Tech Mono', monospace;
    --font-ui:   'Exo 2', sans-serif;
  }

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    background: var(--bg);
    color: var(--text);
    font-family: var(--font-ui);
    min-height: 100vh;
    overflow-x: hidden;
  }

  /* === GRID BACKGROUND === */
  body::before {
    content: '';
    position: fixed; inset: 0;
    background-image:
      linear-gradient(rgba(0,229,255,0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(0,229,255,0.03) 1px, transparent 1px);
    background-size: 40px 40px;
    pointer-events: none;
    z-index: 0;
  }

  /* === TOP SCANLINE === */
  body::after {
    content: '';
    position: fixed; top: 0; left: 0; right: 0;
    height: 2px;
    background: linear-gradient(90deg, transparent, var(--accent), transparent);
    animation: scanline 3s linear infinite;
    z-index: 100;
    opacity: 0.6;
  }
  @keyframes scanline {
    0%   { transform: scaleX(0); transform-origin: left; }
    50%  { transform: scaleX(1); transform-origin: left; }
    51%  { transform: scaleX(1); transform-origin: right; }
    100% { transform: scaleX(0); transform-origin: right; }
  }

  /* === HEADER === */
  header {
    position: relative; z-index: 1;
    display: flex; align-items: center; justify-content: space-between;
    padding: 18px 30px;
    border-bottom: 1px solid var(--border);
    background: rgba(13,21,32,0.9);
    backdrop-filter: blur(10px);
  }
  .logo {
    display: flex; align-items: center; gap: 14px;
  }
  .logo-icon {
    width: 38px; height: 38px;
    border: 2px solid var(--accent);
    border-radius: 8px;
    display: flex; align-items: center; justify-content: center;
    font-size: 20px;
    box-shadow: var(--glow), inset 0 0 10px rgba(0,229,255,0.1);
    animation: pulse-border 2s ease-in-out infinite;
  }
  @keyframes pulse-border {
    0%,100% { box-shadow: var(--glow), inset 0 0 10px rgba(0,229,255,0.1); }
    50%      { box-shadow: 0 0 20px rgba(0,229,255,0.6), inset 0 0 14px rgba(0,229,255,0.2); }
  }
  .logo-text { font-size: 0.7rem; font-weight: 300; color: var(--muted); letter-spacing: 3px; text-transform: uppercase; }
  .logo-title { font-size: 1.1rem; font-weight: 800; color: var(--text); letter-spacing: 1px; }
  .header-right { display: flex; align-items: center; gap: 16px; }
  .live-badge {
    display: flex; align-items: center; gap: 7px;
    font-family: var(--font-mono); font-size: 0.72rem;
    color: var(--green); letter-spacing: 2px;
  }
  .live-dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    animation: blink 1.2s ease-in-out infinite;
  }
  @keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.2} }
  .refresh-note {
    font-family: var(--font-mono); font-size: 0.65rem;
    color: var(--muted);
  }

  /* === MAIN LAYOUT === */
  .container {
    position: relative; z-index: 1;
    max-width: 1280px; margin: 0 auto;
    padding: 24px 24px 48px;
  }

  /* === SECTION LABELS === */
  .section-label {
    font-family: var(--font-mono); font-size: 0.65rem;
    color: var(--accent); letter-spacing: 4px; text-transform: uppercase;
    margin-bottom: 14px;
    display: flex; align-items: center; gap: 10px;
  }
  .section-label::after {
    content: ''; flex: 1; height: 1px;
    background: linear-gradient(90deg, var(--border), transparent);
  }

  /* === STATUS BANNER === */
  .status-banner {
    display: flex; align-items: center; justify-content: space-between;
    gap: 16px;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 20px 28px;
    margin-bottom: 28px;
    overflow: hidden;
    position: relative;
  }
  .status-banner.occupied { border-color: rgba(0,255,136,0.4); }
  .status-banner.empty    { border-color: rgba(255,59,92,0.3); }
  .status-banner::before {
    content: '';
    position: absolute; left: 0; top: 0; bottom: 0; width: 4px;
  }
  .status-banner.occupied::before { background: var(--green); box-shadow: 0 0 12px var(--green); }
  .status-banner.empty::before    { background: var(--red);   box-shadow: 0 0 12px var(--red); }

  .occ-label { font-size: 0.7rem; letter-spacing: 3px; text-transform: uppercase; color: var(--muted); }
  .occ-status { font-size: 2.2rem; font-weight: 800; line-height: 1; margin-top: 4px; }
  .occ-status.occ { color: var(--green); text-shadow: 0 0 20px rgba(0,255,136,0.5); }
  .occ-status.emp { color: var(--red);   text-shadow: 0 0 20px rgba(255,59,92,0.4); }

  .banner-meta { display: flex; gap: 28px; }
  .meta-item { text-align: right; }
  .meta-key { font-size: 0.65rem; letter-spacing: 2px; text-transform: uppercase; color: var(--muted); }
  .meta-val { font-family: var(--font-mono); font-size: 1.1rem; color: var(--text); margin-top: 2px; }

  /* === SENSOR GRID === */
  .sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 16px;
    margin-bottom: 28px;
  }
  .sensor-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 20px;
    position: relative;
    overflow: hidden;
    transition: border-color 0.3s;
  }
  .sensor-card::after {
    content: '';
    position: absolute; bottom: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--accent), transparent);
    opacity: 0;
    transition: opacity 0.3s;
  }
  .sensor-card:hover { border-color: rgba(0,229,255,0.35); }
  .sensor-card:hover::after { opacity: 1; }

  .sensor-icon { font-size: 1.4rem; margin-bottom: 12px; }
  .sensor-label { font-size: 0.65rem; letter-spacing: 3px; text-transform: uppercase; color: var(--muted); margin-bottom: 8px; }
  .sensor-value {
    font-family: var(--font-mono); font-size: 1.8rem; font-weight: 400;
    color: var(--accent);
    line-height: 1;
  }
  .sensor-unit { font-size: 0.8rem; color: var(--muted); margin-left: 4px; }
  .sensor-status-pill {
    display: inline-block;
    margin-top: 8px;
    padding: 3px 10px;
    border-radius: 20px;
    font-size: 0.65rem; font-family: var(--font-mono); letter-spacing: 1px;
  }
  .pill-on  { background: rgba(0,255,136,0.12); color: var(--green); border: 1px solid rgba(0,255,136,0.3); }
  .pill-off { background: rgba(255,59,92,0.12);  color: var(--red);   border: 1px solid rgba(255,59,92,0.3); }

  /* === BOTTOM ROW === */
  .bottom-row {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 20px;
  }
  @media (max-width: 760px) { .bottom-row { grid-template-columns: 1fr; } }

  /* === CONTROL PANEL === */
  .control-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 24px;
  }
  .ctrl-header {
    display: flex; justify-content: space-between; align-items: center;
    margin-bottom: 20px;
  }
  .ctrl-title { font-weight: 600; font-size: 1rem; }
  .mode-badge {
    font-family: var(--font-mono); font-size: 0.68rem; letter-spacing: 2px;
    padding: 4px 12px; border-radius: 20px;
  }
  .mode-auto   { background: rgba(0,229,255,0.1); color: var(--accent); border: 1px solid rgba(0,229,255,0.3); }
  .mode-manual { background: rgba(255,149,0,0.1); color: var(--orange); border: 1px solid rgba(255,149,0,0.3); }

  .ctrl-buttons { display: flex; flex-direction: column; gap: 12px; }
  .ctrl-btn {
    display: block; width: 100%;
    padding: 14px 20px;
    border: 1px solid;
    border-radius: 10px;
    font-family: var(--font-ui); font-size: 0.85rem; font-weight: 600;
    letter-spacing: 2px; text-transform: uppercase;
    text-decoration: none; text-align: center;
    cursor: pointer;
    transition: all 0.2s;
    position: relative; overflow: hidden;
  }
  .ctrl-btn::before {
    content: '';
    position: absolute; inset: 0;
    background: currentColor;
    opacity: 0;
    transition: opacity 0.2s;
  }
  .ctrl-btn:hover::before { opacity: 0.07; }

  .btn-on   { color: var(--green);  border-color: rgba(0,255,136,0.4);  background: rgba(0,255,136,0.06); }
  .btn-off  { color: var(--red);    border-color: rgba(255,59,92,0.4);   background: rgba(255,59,92,0.06); }
  .btn-auto { color: var(--accent); border-color: rgba(0,229,255,0.4);  background: rgba(0,229,255,0.06); }

  .btn-on:hover   { box-shadow: 0 0 16px rgba(0,255,136,0.25); }
  .btn-off:hover  { box-shadow: 0 0 16px rgba(255,59,92,0.25); }
  .btn-auto:hover { box-shadow: 0 0 16px rgba(0,229,255,0.25); }

  .btn-icon { margin-right: 8px; }

  /* === RELAY STATUS CARD === */
  .relay-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 24px;
    display: flex; flex-direction: column; justify-content: space-between;
  }
  .relay-visual {
    display: flex; align-items: center; justify-content: center;
    flex: 1; padding: 24px 0;
  }
  .relay-circle {
    width: 110px; height: 110px;
    border-radius: 50%;
    display: flex; flex-direction: column; align-items: center; justify-content: center;
    gap: 4px;
    border: 3px solid;
    font-family: var(--font-mono);
  }
  .relay-circle.on  {
    border-color: var(--green);
    box-shadow: 0 0 30px rgba(0,255,136,0.3), inset 0 0 20px rgba(0,255,136,0.1);
    animation: relay-pulse 2s ease-in-out infinite;
  }
  .relay-circle.off {
    border-color: var(--muted);
    opacity: 0.5;
  }
  @keyframes relay-pulse {
    0%,100% { box-shadow: 0 0 30px rgba(0,255,136,0.3), inset 0 0 20px rgba(0,255,136,0.1); }
    50%      { box-shadow: 0 0 50px rgba(0,255,136,0.5), inset 0 0 30px rgba(0,255,136,0.15); }
  }
  .relay-emoji { font-size: 2rem; }
  .relay-text  { font-size: 0.8rem; letter-spacing: 3px; }
  .relay-text.on  { color: var(--green); }
  .relay-text.off { color: var(--muted); }

  .relay-info { font-size: 0.7rem; color: var(--muted); text-align: center; letter-spacing: 1px; margin-top: 8px; }

  /* === CAMERA SECTION === */
  .camera-section {
    margin-top: 28px;
  }
  .camera-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 14px;
    overflow: hidden;
  }
  .camera-header {
    display: flex; align-items: center; justify-content: space-between;
    padding: 16px 24px;
    border-bottom: 1px solid var(--border);
    background: rgba(0,0,0,0.3);
  }
  .camera-title {
    display: flex; align-items: center; gap: 10px;
    font-weight: 600; font-size: 0.95rem;
  }
  .cam-live-dot {
    width: 9px; height: 9px; border-radius: 50%;
    background: var(--red);
    box-shadow: 0 0 8px var(--red);
    animation: blink 1s ease-in-out infinite;
  }
  .camera-controls {
    display: flex; gap: 10px;
  }
  .cam-ctrl-btn {
    font-family: var(--font-mono); font-size: 0.68rem;
    letter-spacing: 1px;
    padding: 5px 12px; border-radius: 6px;
    border: 1px solid var(--border);
    background: rgba(255,255,255,0.04);
    color: var(--muted);
    cursor: pointer;
    transition: all 0.2s;
    text-decoration: none;
  }
  .cam-ctrl-btn:hover { border-color: var(--accent); color: var(--accent); }
  .cam-ctrl-btn.active { border-color: var(--accent); color: var(--accent); background: rgba(0,229,255,0.08); }

  .camera-body { position: relative; }

  /* Stream view */
  .stream-view {
    display: none;
    padding: 20px;
    background: #000;
  }
  .stream-view.active { display: block; }
  .stream-view img {
    width: 100%;
    border-radius: 8px;
    display: block;
    border: 1px solid var(--border);
  }
  .stream-overlay {
    position: absolute;
    top: 30px; left: 30px;
    display: flex; align-items: center; gap: 7px;
    font-family: var(--font-mono); font-size: 0.65rem; letter-spacing: 2px;
    color: var(--red);
    pointer-events: none;
  }
  .rec-dot {
    width: 7px; height: 7px; border-radius: 50%;
    background: var(--red); box-shadow: 0 0 6px var(--red);
    animation: blink 0.8s ease-in-out infinite;
  }

  /* Snapshot view */
  .snapshot-view {
    display: none;
    padding: 20px;
    background: #000;
    text-align: center;
  }
  .snapshot-view.active { display: block; }
  .snapshot-view img {
    max-width: 100%; max-height: 400px;
    border-radius: 8px;
    border: 1px solid var(--border);
  }
  .snapshot-placeholder {
    padding: 60px 20px;
    color: var(--muted);
    font-family: var(--font-mono);
    font-size: 0.8rem;
    letter-spacing: 2px;
  }
  .snap-icon { font-size: 3rem; margin-bottom: 16px; opacity: 0.3; }
  .snap-btn {
    margin-top: 20px;
    display: inline-block;
    padding: 10px 24px;
    border: 1px solid var(--accent);
    border-radius: 8px;
    color: var(--accent);
    font-family: var(--font-mono);
    font-size: 0.75rem;
    letter-spacing: 2px;
    cursor: pointer;
    background: rgba(0,229,255,0.06);
    transition: all 0.2s;
    text-decoration: none;
  }
  .snap-btn:hover { background: rgba(0,229,255,0.14); box-shadow: var(--glow); }

  /* Detection view */
  .detection-view {
    display: none;
    padding: 24px;
  }
  .detection-view.active { display: block; }
  .det-status-row {
    display: flex; align-items: center; gap: 16px;
    padding: 20px;
    background: rgba(0,0,0,0.3);
    border-radius: 10px;
    border: 1px solid var(--border);
    margin-bottom: 16px;
  }
  .det-icon { font-size: 2.5rem; }
  .det-label { font-size: 0.65rem; letter-spacing: 3px; text-transform: uppercase; color: var(--muted); }
  .det-result { font-size: 1.5rem; font-weight: 700; margin-top: 4px; }
  .det-result.human { color: var(--green); text-shadow: 0 0 16px rgba(0,255,136,0.4); }
  .det-result.empty { color: var(--red); }
  .det-meta {
    display: grid; grid-template-columns: 1fr 1fr; gap: 12px;
  }
  .det-meta-item {
    background: rgba(0,0,0,0.3);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 14px;
    font-family: var(--font-mono);
  }
  .det-meta-label { font-size: 0.62rem; letter-spacing: 2px; color: var(--muted); text-transform: uppercase; }
  .det-meta-val   { font-size: 1rem; color: var(--text); margin-top: 4px; }

  /* Camera footer */
  .camera-footer {
    display: flex; align-items: center; justify-content: space-between;
    padding: 12px 20px;
    border-top: 1px solid var(--border);
    background: rgba(0,0,0,0.2);
    font-family: var(--font-mono); font-size: 0.65rem; color: var(--muted);
    letter-spacing: 1px;
  }
  .cam-ip { color: var(--accent); }

  /* === FOOTER === */
  .page-footer {
    position: relative; z-index: 1;
    text-align: center;
    padding: 24px;
    font-family: var(--font-mono); font-size: 0.65rem;
    color: var(--muted); letter-spacing: 2px;
    border-top: 1px solid var(--border);
  }

  /* === ANIMATIONS ON LOAD === */
  .fade-in { animation: fadeUp 0.5s ease both; }
  .fade-in:nth-child(1) { animation-delay: 0.05s; }
  .fade-in:nth-child(2) { animation-delay: 0.10s; }
  .fade-in:nth-child(3) { animation-delay: 0.15s; }
  .fade-in:nth-child(4) { animation-delay: 0.20s; }
  .fade-in:nth-child(5) { animation-delay: 0.25s; }
  @keyframes fadeUp {
    from { opacity: 0; transform: translateY(16px); }
    to   { opacity: 1; transform: translateY(0); }
  }
</style>
</head>
<body>

<!-- ===== HEADER ===== -->
<header>
  <div class="logo">
    <div class="logo-icon">⚡</div>
    <div>
      <div class="logo-text">AI-Based System</div>
      <div class="logo-title">Smart Energy Monitor</div>
    </div>
  </div>
  <div class="header-right">
    <div class="live-badge"><div class="live-dot"></div> LIVE</div>
    <div class="refresh-note">AUTO-REFRESH 3s</div>
  </div>
</header>

<!-- ===== MAIN CONTENT ===== -->
<div class="container">

  <!-- Occupancy Banner -->
  <div class="section-label">▸ Room Status</div>
  <div class="status-banner )rawhtml";

    html += relayOn ? "occupied" : "empty";
    html += R"rawhtml( fade-in">
    <div>
      <div class="occ-label">Occupancy Detection</div>
      <div class="occ-status )rawhtml";

    html += humanDetected ? "occ\">OCCUPIED" : "emp\">EMPTY";

    html += R"rawhtml(</div>
    </div>
    <div class="banner-meta">
      <div class="meta-item">
        <div class="meta-key">PIR Motion</div>
        <div class="meta-val">)rawhtml";
    html += pirState == HIGH ? "ACTIVE" : "IDLE";
    html += R"rawhtml(</div>
      </div>
      <div class="meta-item">
        <div class="meta-key">Distance</div>
        <div class="meta-val">)rawhtml";
    html += String(distance, 1);
    html += R"rawhtml( cm</div>
      </div>
      <div class="meta-item">
        <div class="meta-key">Power Draw</div>
        <div class="meta-val">)rawhtml";
    html += String(power, 1);
    html += R"rawhtml( W</div>
      </div>
    </div>
  </div>

  <!-- Sensor Grid -->
  <div class="section-label">▸ Sensor Readings</div>
  <div class="sensor-grid">

    <div class="sensor-card fade-in">
      <div class="sensor-icon">🔍</div>
      <div class="sensor-label">PIR Motion</div>
      <div class="sensor-value" style="font-size:1.3rem;">)rawhtml";
    html += pirState == HIGH ? "<span style='color:var(--green)'>MOTION</span>" : "<span style='color:var(--muted)'>IDLE</span>";
    html += R"rawhtml(</div>
      <span class="sensor-status-pill )rawhtml";
    html += pirState == HIGH ? "pill-on\">● ACTIVE" : "pill-off\">● INACTIVE";
    html += R"rawhtml(</span>
    </div>

    <div class="sensor-card fade-in">
      <div class="sensor-icon">📡</div>
      <div class="sensor-label">Ultrasonic Distance</div>
      <div class="sensor-value">)rawhtml";
    html += String(distance, 1);
    html += R"rawhtml(<span class="sensor-unit">cm</span></div>
      <span class="sensor-status-pill )rawhtml";
    html += distance < 100 ? "pill-on\">● IN RANGE" : "pill-off\">● OUT OF RANGE";
    html += R"rawhtml(</span>
    </div>

    <div class="sensor-card fade-in">
      <div class="sensor-icon">⚡</div>
      <div class="sensor-label">Current (ACS712)</div>
      <div class="sensor-value">)rawhtml";
    html += String(current, 3);
    html += R"rawhtml(<span class="sensor-unit">A</span></div>
      <span class="sensor-status-pill )rawhtml";
    html += current > 0.1 ? "pill-on\">● LOAD ACTIVE" : "pill-off\">● NO LOAD";
    html += R"rawhtml(</span>
    </div>

    <div class="sensor-card fade-in">
      <div class="sensor-icon">🔋</div>
      <div class="sensor-label">Power Consumption</div>
      <div class="sensor-value">)rawhtml";
    html += String(power, 1);
    html += R"rawhtml(<span class="sensor-unit">W</span></div>
      <span class="sensor-status-pill )rawhtml";
    html += power > 0 ? "pill-on\">● CONSUMING" : "pill-off\">● ZERO DRAW";
    html += R"rawhtml(</span>
    </div>

    <div class="sensor-card fade-in">
      <div class="sensor-icon">🤖</div>
      <div class="sensor-label">Face Detection (CAM)</div>
      <div class="sensor-value" style="font-size:1.3rem;">)rawhtml";
    html += humanDetected ? "<span style='color:var(--green)'>DETECTED</span>" : "<span style='color:var(--muted)'>NONE</span>";
    html += R"rawhtml(</div>
      <span class="sensor-status-pill )rawhtml";
    html += humanDetected ? "pill-on\">● HUMAN PRESENT" : "pill-off\">● NO HUMAN";
    html += R"rawhtml(</span>
    </div>

  </div><!-- /sensor-grid -->

  <!-- Bottom Row: Control + Relay -->
  <div class="section-label">▸ Control Panel</div>
  <div class="bottom-row">

    <!-- Control Card -->
    <div class="control-card fade-in">
      <div class="ctrl-header">
        <div class="ctrl-title">Relay Control</div>
        <div class="mode-badge )rawhtml";
    html += manualMode ? "mode-manual\">⚙ MANUAL" : "mode-auto\">◉ AUTO";
    html += R"rawhtml(</div>
      </div>
      <div class="ctrl-buttons">
        <a href="/relayon"  class="ctrl-btn btn-on" ><span class="btn-icon">●</span> Turn ON</a>
        <a href="/relayoff" class="ctrl-btn btn-off"><span class="btn-icon">●</span> Turn OFF</a>
        <a href="/automode" class="ctrl-btn btn-auto"><span class="btn-icon">◉</span> Auto Mode</a>
      </div>
    </div>

    <!-- Relay Status Card -->
    <div class="relay-card fade-in">
      <div style="font-weight:600; font-size:0.95rem;">Relay Status</div>
      <div class="relay-visual">
        <div class="relay-circle )rawhtml";
    html += relayOn ? "on\">" : "off\">";
    html += R"rawhtml(
          <div class="relay-emoji">)rawhtml";
    html += relayOn ? "⚡" : "○";
    html += R"rawhtml(</div>
          <div class="relay-text )rawhtml";
    html += relayOn ? "on\">ON" : "off\">OFF";
    html += R"rawhtml(</div>
        </div>
      </div>
      <div class="relay-info">)rawhtml";
    html += relayOn ? "AC LOAD ACTIVE — RELAY CLOSED" : "AC LOAD OFF — RELAY OPEN";
    html += R"rawhtml(</div>
    </div>

  </div><!-- /bottom-row -->

  <!-- ========== CAMERA SECTION ========== -->
  <div class="camera-section">
    <div class="section-label">▸ ESP32-CAM Live Viewer</div>

    <div class="camera-card fade-in">
      <div class="camera-header">
        <div class="camera-title">
          <div class="cam-live-dot"></div>
          Camera Feed
        </div>
        <div class="camera-controls">
          <button class="cam-ctrl-btn active" id="btnStream"   onclick="showTab('stream')">📹 Stream</button>
          <button class="cam-ctrl-btn"        id="btnSnap"     onclick="showTab('snap')">📷 Snapshot</button>
          <button class="cam-ctrl-btn"        id="btnDetect"   onclick="showTab('detect')">🤖 Detection</button>
        </div>
      </div>

      <div class="camera-body">

        <!-- STREAM TAB -->
        <div class="stream-view active" id="tabStream">
          <div class="stream-overlay"><div class="rec-dot"></div> REC</div>
          <img id="streamImg"
               src="http://10.42.156.3/stream"
               alt="ESP32-CAM Stream"
               onerror="this.src=''; this.parentElement.innerHTML='<div style=\'padding:60px;text-align:center;font-family:monospace;color:#4a6a85;font-size:0.8rem;letter-spacing:2px;\'>📷<br><br>STREAM UNAVAILABLE<br><br><span style=\'color:#1a2d45;\'>CHECK ESP32-CAM IP</span></div>'"
          />
        </div>

        <!-- SNAPSHOT TAB -->
        <div class="snapshot-view" id="tabSnap">
          <div class="snapshot-placeholder">
            <div class="snap-icon">📷</div>
            <div>TAP TO CAPTURE SNAPSHOT</div>
            <div style="margin-top:8px;font-size:0.65rem;color:#1a2d45;">FROM ESP32-CAM MODULE</div>
          </div>
          <a href="http://10.42.156.3/capture" target="_blank" class="snap-btn">⬛ CAPTURE FRAME</a>
        </div>

        <!-- DETECTION TAB -->
        <div class="detection-view" id="tabDetect">
          <div class="det-status-row">
            <div class="det-icon">)rawhtml";
    html += humanDetected ? "🧑" : "👤";
    html += R"rawhtml(</div>
            <div>
              <div class="det-label">Face Detection Result</div>
              <div class="det-result )rawhtml";
    html += humanDetected ? "human\">✓ HUMAN DETECTED" : "empty\">✗ NO HUMAN DETECTED";
    html += R"rawhtml(</div>
            </div>
          </div>
          <div class="det-meta">
            <div class="det-meta-item">
              <div class="det-meta-label">CAM Endpoint</div>
              <div class="det-meta-val" style="font-size:0.8rem;">ESP32-CAM → /human</div>
            </div>
            <div class="det-meta-item">
              <div class="det-meta-label">Last Signal</div>
              <div class="det-meta-val">)rawhtml";
    html += humanDetected ? "HUMAN" : "CLEAR";
    html += R"rawhtml(</div>
            </div>
            <div class="det-meta-item">
              <div class="det-meta-label">PIR Confirm</div>
              <div class="det-meta-val">)rawhtml";
    html += pirState == HIGH ? "✓ YES" : "✗ NO";
    html += R"rawhtml(</div>
            </div>
            <div class="det-meta-item">
              <div class="det-meta-label">Ultrasonic Confirm</div>
              <div class="det-meta-val">)rawhtml";
    html += distance < 100 ? "✓ IN RANGE" : "✗ OUT";
    html += R"rawhtml(</div>
            </div>
          </div>
        </div>

      </div><!-- /camera-body -->

      <div class="camera-footer">
        <span>ESP32-CAM</span>
        <span class="cam-ip">▸ 192.168.1.101</span>
        <span>OV2640 SENSOR</span>
      </div>
    </div>
  </div><!-- /camera-section -->

</div><!-- /container -->

<footer class="page-footer">
  SMART ENERGY WASTAGE DETECTION SYSTEM &nbsp;|&nbsp; ESP32 + ESP32-CAM &nbsp;|&nbsp; IoT v1.0
</footer>

<script>
  function showTab(tab) {
    document.getElementById('tabStream').classList.remove('active');
    document.getElementById('tabSnap').classList.remove('active');
    document.getElementById('tabDetect').classList.remove('active');
    document.getElementById('btnStream').classList.remove('active');
    document.getElementById('btnSnap').classList.remove('active');
    document.getElementById('btnDetect').classList.remove('active');

    if (tab === 'stream') {
      document.getElementById('tabStream').classList.add('active');
      document.getElementById('btnStream').classList.add('active');
    } else if (tab === 'snap') {
      document.getElementById('tabSnap').classList.add('active');
      document.getElementById('btnSnap').classList.add('active');
    } else {
      document.getElementById('tabDetect').classList.add('active');
      document.getElementById('btnDetect').classList.add('active');
    }
  }
</script>

</body>
</html>
)rawhtml";

    server.send(200, "text/html", html);
}

// ==========================
// SETUP
// ==========================

void setup()
{
    Serial.begin(74880);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    pinMode(PIR_PIN, INPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    analogReadResolution(12);

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi Connected!");

    Serial.print("ESP32 Dashboard IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);

    server.on("/human", handleHuman);
    server.on("/nohuman", handleNoHuman);

    server.on("/relayon", handleRelayOn);
    server.on("/relayoff", handleRelayOff);
    server.on("/automode", handleAutoMode);

    server.begin();

    Serial.println("Server Started");
}

// ==========================
// LOOP
// ==========================

void loop()
{
    server.handleClient();

    // ==========================
    // PIR SENSOR
    // ==========================

    pirState = digitalRead(PIR_PIN);

    // ==========================
    // ULTRASONIC SENSOR
    // ==========================

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);

    distance = duration * 0.034 / 2;

    // ==========================
    // CURRENT SENSOR
    // ==========================

    int adcValue = analogRead(CURRENT_SENSOR);

    float voltage = (adcValue / 4095.0) * 3.3;

    current = abs((voltage - 2.5) / 0.185);

    if(current < 0.10)
    {
        current = 0;
    }

    power = current * 230.0;

    // ==========================
    // AUTO MODE LOGIC
    // ==========================

    if(!manualMode)
    {
        if(humanDetected == true &&
           pirState == HIGH &&
           distance < 100)
        {
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("ROOM OCCUPIED");
        }
        else
        {
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("ROOM EMPTY");
        }
    }

    // ==========================
    // SERIAL MONITOR
    // ==========================

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    Serial.print("Current: ");
    Serial.print(current);
    Serial.println(" A");

    Serial.print("Power: ");
    Serial.print(power);
    Serial.println(" W");

    Serial.println("-----------------------");

    delay(5000);
}