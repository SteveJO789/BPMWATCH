#include <Arduino.h>
#include <DW1000Ranging.h>
#include <SPI.h>

// Set to 1 for Master/Anchor, or 0 for Slave 1/Tag.
#define UWB_IS_MASTER 1

#if UWB_IS_MASTER
#include <WebServer.h>
#include <WiFi.h>
#endif

constexpr int UWB_SCK = 18;
constexpr int UWB_MISO = 19;
constexpr int UWB_MOSI = 23;
constexpr int UWB_CS = 5;
constexpr int UWB_IRQ = 34;
constexpr int UWB_RST = 4;

char MASTER_EUI[] = "0C:8A:D3:7C:E5:A4:00:01";
char TAG_EUI[] = "1C:75:C4:F4:E9:D4:00:01";

float lastDistanceM = 0.0f;
float lastRxPowerDbm = 0.0f;
float lastQuality = 0.0f;
uint32_t lastRangeMs = 0;
uint32_t rangeCount = 0;
bool peerPresent = false;

#if UWB_IS_MASTER
constexpr char AP_SSID[] = "S.T.A.T-UWB";
constexpr char AP_PASSWORD[] = "statuwb123";
WebServer server(80);

const char MONITOR_PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>S.T.A.T UWB Monitor</title>
  <style>
    body{font-family:Arial,sans-serif;background:#101820;color:#f2f5f7;margin:0;padding:24px}
    main{max-width:560px;margin:auto;background:#1b2835;border-radius:14px;padding:24px}
    h1{margin-top:0;color:#45d483}.status{font-weight:bold}.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}
    .card{background:#263849;border-radius:10px;padding:16px}.value{font-size:1.8rem;margin-top:6px}
    .ok{color:#45d483}.lost{color:#ff6b6b}small{color:#aab7c4}
  </style>
</head>
<body><main>
  <h1>S.T.A.T UWB Monitor</h1>
  <p>Pair status: <span id="status" class="status lost">WAITING</span></p>
  <div class="grid">
    <div class="card"><small>Distance</small><div id="distance" class="value">-- m</div></div>
    <div class="card"><small>RX power</small><div id="rx" class="value">-- dBm</div></div>
    <div class="card"><small>Quality</small><div id="quality" class="value">--</div></div>
    <div class="card"><small>Range updates</small><div id="count" class="value">0</div></div>
  </div>
  <p><small id="age">No range received</small></p>
</main>
<script>
async function update(){
  try{
    const r=await fetch('/api/status',{cache:'no-store'}),d=await r.json();
    const s=document.getElementById('status');
    s.textContent=d.connected?'CONNECTED':'WAITING';
    s.className='status '+(d.connected?'ok':'lost');
    document.getElementById('distance').textContent=d.hasRange?d.distance_m.toFixed(2)+' m':'-- m';
    document.getElementById('rx').textContent=d.hasRange?d.rx_power_dbm.toFixed(1)+' dBm':'-- dBm';
    document.getElementById('quality').textContent=d.hasRange?d.quality.toFixed(1):'--';
    document.getElementById('count').textContent=d.range_count;
    document.getElementById('age').textContent=d.hasRange?'Last update '+d.age_ms+' ms ago':'No range received';
  }catch(e){document.getElementById('status').textContent='WEB ERROR'}
}
setInterval(update,500);update();
</script></body></html>
)HTML";
#endif

void onNewRange() {
  DW1000Device *device = DW1000Ranging.getDistantDevice();
  if (device == nullptr) {
    return;
  }

  lastDistanceM = device->getRange();
  lastRxPowerDbm = device->getRXPower();
  lastQuality = device->getQuality();
  lastRangeMs = millis();
  rangeCount++;
  peerPresent = true;

  Serial.printf("Range #%lu: %.2f m, RX %.1f dBm, quality %.1f\n",
                static_cast<unsigned long>(rangeCount), lastDistanceM,
                lastRxPowerDbm, lastQuality);
}

void onNewDevice(DW1000Device *device) {
  peerPresent = true;
  Serial.printf("UWB peer connected: short address 0x%04X\n",
                device->getShortAddress());
}

void onInactiveDevice(DW1000Device *device) {
  peerPresent = false;
  Serial.printf("UWB peer inactive: short address 0x%04X\n",
                device->getShortAddress());
}

#if UWB_IS_MASTER
void sendStatusJson() {
  const uint32_t now = millis();
  const bool hasRange = rangeCount > 0;
  const uint32_t ageMs = hasRange ? now - lastRangeMs : 0;
  const bool connected = peerPresent && hasRange && ageMs < 3000;

  char json[256];
  snprintf(json, sizeof(json),
           "{\"connected\":%s,\"hasRange\":%s,\"distance_m\":%.2f,"
           "\"rx_power_dbm\":%.1f,\"quality\":%.1f,\"range_count\":%lu,"
           "\"age_ms\":%lu}",
           connected ? "true" : "false", hasRange ? "true" : "false",
           lastDistanceM, lastRxPowerDbm, lastQuality,
           static_cast<unsigned long>(rangeCount),
           static_cast<unsigned long>(ageMs));

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void startMonitorAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", []() { server.send_P(200, "text/html", MONITOR_PAGE); });
  server.on("/api/status", sendStatusJson);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();

  Serial.printf("Wi-Fi AP: %s\n", AP_SSID);
  Serial.printf("Password: %s\n", AP_PASSWORD);
  Serial.printf("Monitor: http://%s\n", WiFi.softAPIP().toString().c_str());
}
#endif

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("S.T.A.T real UWB pair ranging test");

#if UWB_IS_MASTER
  Serial.println("Role: MASTER / ANCHOR");
  startMonitorAccessPoint();
#else
  Serial.println("Role: SLAVE 1 / TAG");
#endif

  SPI.begin(UWB_SCK, UWB_MISO, UWB_MOSI, UWB_CS);
  DW1000Ranging.initCommunication(UWB_RST, UWB_CS, UWB_IRQ);
  DW1000Ranging.attachNewRange(onNewRange);
  DW1000Ranging.attachNewDevice(onNewDevice);
  DW1000Ranging.attachInactiveDevice(onInactiveDevice);
  DW1000Ranging.useRangeFilter(true);
  DW1000Ranging.setRangeFilterValue(5);

#if UWB_IS_MASTER
  DW1000Ranging.startAsAnchor(MASTER_EUI,
                              DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
#else
  DW1000Ranging.startAsTag(TAG_EUI, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                           false);
#endif
}

void loop() {
  DW1000Ranging.loop();
#if UWB_IS_MASTER
  server.handleClient();
#endif
}
