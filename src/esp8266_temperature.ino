#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

// --- WEB STATUS ADD-ON BEGIN ---
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
float lastTemperatureWeb = NAN;  // für Webseite
// --- WEB STATUS ADD-ON END ---

/* ================= ntfy ================= */
const char* NTFY_HOST  = "ntfy.sh";
const int   NTFY_PORT  = 443;
const char* NTFY_TOPIC = "dojo-psc-tSztbudCJbJgNRUm";

/* ============ Defaults ================= */
const float DEFAULT_TEMP_WARN     = 20.0;
const float DEFAULT_TEMP_CRITICAL = 18.0;

/* ============ EEPROM ================= */
#define EEPROM_SIZE 64
#define ADDR_WARN      0
#define ADDR_CRITICAL  sizeof(float)
#define ADDR_HEARTBEAT 16  // für täglichen Heartbeat

/* ============ Temperaturgrenzen ================= */
float TEMP_WARN     = DEFAULT_TEMP_WARN;
float TEMP_CRITICAL = DEFAULT_TEMP_CRITICAL;

/* ================= Hardware ================= */
#define ONE_WIRE_BUS D2

#define LED_RED    D5
#define LED_YELLOW D6
#define LED_GREEN  D7

#define BTN_PIN D1
const unsigned long LONG_PRESS_MS = 3000;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClientSecure client;

/* ================= Zustände ================= */
enum TempState {
  STATE_OK,
  STATE_WARN,
  STATE_CRITICAL
};
TempState lastState = STATE_OK;

/* ================= Timing ================= */
unsigned long lastCheck = 0;
const unsigned long CHECK_INTERVAL = 300UL * 1000UL;  // Temperaturintervall 5 Minuten

/* ================= Button ================= */
unsigned long btnDownSince = 0;

/* ================= Push Blink ================= */
bool pushBlinkActive = false;
int  pushBlinkCount = 0;
unsigned long lastBlinkToggle = 0;
bool blinkState = false;

/* ================= Heartbeat ================= */
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 24UL*60UL*60UL*1000UL; // 24 Stunden

/* ================= LED Helpers ================= */
void allLedsOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void showTempState(TempState state) {
  allLedsOff();
  if (state == STATE_OK) digitalWrite(LED_GREEN, HIGH);
  else if (state == STATE_WARN) digitalWrite(LED_YELLOW, HIGH);
  else digitalWrite(LED_RED, HIGH);
}

/* ================= EEPROM ================= */
bool loadThresholds() {
  float w, c;
  EEPROM.get(ADDR_WARN, w);
  EEPROM.get(ADDR_CRITICAL, c);

  if (isnan(w) || isnan(c)) return false;
  if (c >= w) return false;
  if (w < -20 || w > 50) return false;
  if (c < -20 || c > 50) return false;

  TEMP_WARN = w;
  TEMP_CRITICAL = c;
  return true;
}

void saveThresholds(float w, float c) {
  EEPROM.put(ADDR_WARN, w);
  EEPROM.put(ADDR_CRITICAL, c);
  EEPROM.commit();
}

void EEPROMWriteULong(int address, unsigned long value){
  for(int i=0;i<4;i++){
    EEPROM.write(address+i, (value >> (i*8)) & 0xFF);
  }
  EEPROM.commit();
}

unsigned long EEPROMReadULong(int address){
  unsigned long value = 0;
  for(int i=0;i<4;i++){
    value |= ((unsigned long)EEPROM.read(address+i)) << (i*8);
  }
  return value;
}

/* ================= ntfy Push ================= */
void sendPush(const String& title, const String& message, int priority) {
  Serial.println("[PUSH] Sending ntfy notification");
  Serial.println("  Title: " + title);
  Serial.println("  Message: " + message);

  client.setInsecure();

  if (!client.connect(NTFY_HOST, NTFY_PORT)) {
    Serial.println("[PUSH] Connection failed");
    return;
  }

  client.println("POST /" + String(NTFY_TOPIC) + " HTTP/1.1");
  client.println("Host: " + String(NTFY_HOST));
  client.println("User-Agent: ESP8266");
  client.println("Title: " + title);
  client.println("Priority: " + String(priority));
  client.println("Content-Type: text/plain; charset=utf-8");
  client.println("Content-Length: " + String(message.length()));
  client.println();
  client.print(message);

  delay(50);
  client.stop();

  // Push Blink starten
  pushBlinkActive = true;
  pushBlinkCount = 0;
  blinkState = false;
  lastBlinkToggle = millis();
}

/* ================= Web Status ================= */
// --- WEB STATUS ADD-ON BEGIN ---
String htmlStatusPage() {
  String stateTxt = (lastState == STATE_OK) ? "OK" :
                    (lastState == STATE_WARN) ? "WARNUNG" : "KRITISCH";
  String color = (lastState == STATE_OK) ? "green" :
                 (lastState == STATE_WARN) ? "orange" : "red";

  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";

  // --- Anti-Cache & Auto-Refresh ---
  html += "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'>";
  html += "<meta http-equiv='Pragma' content='no-cache'>";
  html += "<meta http-equiv='Expires' content='0'>";
  html += "<meta http-equiv='refresh' content='300'>";  // alle 5 Minuten aktualisieren

  html += "<title>DojoTemp Status</title></head><body style='font-family:Arial'>";
  html += "<h1>DojoTemp</h1>";
  html += "<p>Status: <b style='color:" + color + "'>" + stateTxt + "</b></p>";
  html += "<p>Temperatur: " + (isnan(lastTemperatureWeb) ? String("n/a") :
           String(lastTemperatureWeb,1) + " &deg;C") + "</p>";
  html += "<p>Warn: " + String(TEMP_WARN,1) + " &deg;C</p>";
  html += "<p>Kritisch: " + String(TEMP_CRITICAL,1) + " &deg;C</p>";
  html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Uptime: " + String(millis()/1000) + " s</p>";
  html += "</body></html>";
  return html;
}
// --- WEB STATUS ADD-ON END ---

/* ================= Setup ================= */
void setup() {
  Serial.begin(115200);
  delay(500);

  // --- Infozeile beibehalten ---
  Serial.printf(
    "\n\nSketch: %s\nBuild: %s\nESP SDK: %s\n\n",
    __FILE__,
    __TIMESTAMP__,
    ESP.getFullVersion().c_str()
  );

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  allLedsOff();

  // LED Test
  Serial.println("LED Test: alle LEDs gemeinsam für 5 Sekunden an");
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay(5000);
  allLedsOff();

  pinMode(BTN_PIN, INPUT_PULLUP);

  EEPROM.begin(EEPROM_SIZE);

  if (loadThresholds()) {
    Serial.printf("[EEPROM] Loaded thresholds: WARN=%.1f CRIT=%.1f\n",
                  TEMP_WARN, TEMP_CRITICAL);
  } else {
    Serial.println("[EEPROM] Invalid or empty, using defaults");
  }

  lastHeartbeat = EEPROMReadULong(ADDR_HEARTBEAT);

  char warnBuf[8], critBuf[8];
  dtostrf(TEMP_WARN, 4, 1, warnBuf);
  dtostrf(TEMP_CRITICAL, 4, 1, critBuf);

  WiFiManagerParameter p_warn("warn", "Warn threshold (C)", warnBuf, 7);
  WiFiManagerParameter p_crit("crit", "Critical threshold (C)", critBuf, 7);

  WiFiManager wm;
  wm.addParameter(&p_warn);
  wm.addParameter(&p_crit);

  if (!wm.autoConnect("Dojo-Temp","tatami2026")) {
    Serial.println("[WiFi] Failed, restarting");
    delay(5000);   // NICHT sofort rebooten
    ESP.restart();
  }

  float newWarn = atof(p_warn.getValue());
  float newCrit = atof(p_crit.getValue());

  if (newCrit < newWarn) {
    TEMP_WARN = newWarn;
    TEMP_CRITICAL = newCrit;
    saveThresholds(newWarn, newCrit);
    Serial.println("[WiFiManager] Thresholds updated");
  }

  sensors.begin();
  delay(750); // wichtig für DS18B20 Stabilität

  // --- sofortige erste Messung ermöglichen ---
  lastCheck = millis() - CHECK_INTERVAL;

  // --- Webserver starten ---
  server.on("/", [](){
    // --- HTTP-Header gegen Caching ---
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");

    server.send(200, "text/html", htmlStatusPage());
  });
  server.begin();
  Serial.println("[WEB] HTTP server started");

  sendPush(
    "Temperature monitor started",
    "Warn < " + String(TEMP_WARN,1) +
    " C\nCritical < " + String(TEMP_CRITICAL,1) + " C",
    1
  );
}

/* ================= Loop ================= */
void loop() {
  server.handleClient();
  unsigned long now = millis();

  /* ---- Button ---- */
  if (digitalRead(BTN_PIN) == LOW) {
    if (btnDownSince == 0) btnDownSince = now;
    if (now - btnDownSince > LONG_PRESS_MS) {
      Serial.println("[BTN] WiFi reset triggered");
      WiFiManager wm;
      wm.resetSettings();
      ESP.restart();
    }
  } else {
    btnDownSince = 0;
  }

  /* ---- Push Blink ---- */
  if (pushBlinkActive) {
    if (now - lastBlinkToggle > 300) {
      lastBlinkToggle = now;
      blinkState = !blinkState;
      digitalWrite(LED_GREEN, blinkState ? HIGH : LOW);

      if (!blinkState && ++pushBlinkCount >= 3) {
        pushBlinkActive = false;
        showTempState(lastState);
      }
    }
    return;
  }

  /* ---- Temperatur ---- */
  if (now - lastCheck >= CHECK_INTERVAL) {
    lastCheck = now;

    sensors.requestTemperatures();
    delay(1000); // Sensor braucht 1s, lange Leitung
    float temp = sensors.getTempCByIndex(0);

    if (temp == DEVICE_DISCONNECTED_C) {
      Serial.println("[TEMP] Sensor not reachable");
      sendPush("Sensor error", "DS18B20 not reachable", 4);
      return;
    }

    Serial.printf("[TEMP] %.2f C\n", temp);
    lastTemperatureWeb = temp; // für Webstatus

    TempState currentState = STATE_OK;
    if (temp < TEMP_CRITICAL) currentState = STATE_CRITICAL;
    else if (temp < TEMP_WARN) currentState = STATE_WARN;

    if (currentState != lastState) {
      Serial.println("[STATE] Temperature state changed");
      sendPush("Temperature state changed",
               "Current: " + String(temp,1) + " C", 3);
      lastState = currentState;
    }

    showTempState(lastState);

    /* ---- Heartbeat ---- */
    if ((now - lastHeartbeat >= HEARTBEAT_INTERVAL) || (lastHeartbeat == 0)) {
      sendPush(
        "Heartbeat: System OK",
        "Dojo-Temperatur: " + String(temp,1) + " C",
        1
      );
      lastHeartbeat = now;
      EEPROMWriteULong(ADDR_HEARTBEAT, lastHeartbeat);
    }
  }
}
