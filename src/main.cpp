#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include "config.h"

// =====================
// Estado local
// =====================
WiFiClient netClient;
PubSubClient mqtt(netClient);

unsigned long lastTsPost = 0;
bool lastPresence = false;
int s_analise = 0;
int s_manut   = 0;
int s_liber   = 0;

// =====================
// Tópicos MQTT
// =====================
String topicTelemetry() { return String("motttu/telemetry/") + DEVICE_ID; }
String topicCmd()       { return String("motttu/actuators/") + DEVICE_ID + "/set"; }
String topicStatus()    { return String("motttu/status/") + DEVICE_ID; }

// =====================
// Wi‑Fi e NTP
// =====================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" OK");
  configTime(TZ_OFFSET_SECONDS, DST_OFFSET_SECONDS, NTP_SERVER);
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
}

String iso8601Utc() {
  struct tm ti;
  if (getLocalTime(&ti)) {
    char iso[25];
    strftime(iso, sizeof(iso), "%Y-%m-%dT%H:%M:%SZ", &ti);
    return String(iso);
  }
  time_t now = time(nullptr);
  return String((uint32_t)now);
}

// =====================
// MQTT
// =====================
void onMqttMessage(char* topic, byte* payload, unsigned int len) {
  String t = topic;
  String body; body.reserve(len);
  for (unsigned int i=0;i<len;i++) body += (char)payload[i];
  Serial.printf("CMD %s %s\n", t.c_str(), body.c_str());

  if (body.indexOf("{\"led\":1}") >= 0) digitalWrite(LED_PIN, HIGH);
  if (body.indexOf("{\"led\":0}") >= 0) digitalWrite(LED_PIN, LOW);
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  mqtt.setBufferSize(512);

  while (!mqtt.connected()) {
    String cid = String("ESP32-") + DEVICE_ID + "-" + String((uint32_t)esp_random(), HEX);
    if (mqtt.connect(cid.c_str(), nullptr, nullptr, topicStatus().c_str(), 1, true, "offline")) {
      mqtt.publish(topicStatus().c_str(), "online", true);
      mqtt.subscribe(topicCmd().c_str(), 1);
      Serial.println("MQTT OK");
    } else {
      Serial.printf("MQTT fail rc=%d\n", mqtt.state());
      delay(1500);
    }
  }
}

// =====================
// ThingSpeak (opcional)
// =====================
void postThingSpeakSnapshot() {
  if (WiFi.status() != WL_CONNECTED) return;

  int f1=0, f2=0, f3=0;
  if (String(SECTOR) == "analise")      f1 = s_analise;
  else if (String(SECTOR) == "manutencao") f2 = s_manut;
  else if (String(SECTOR) == "liberadas")  f3 = s_liber;

  HTTPClient http;
  WiFiClient wclient;
  http.begin(wclient, TS_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "api_key=" + String(TS_WRITE_KEY) +
                "&field1=" + String(f1) +
                "&field2=" + String(f2) +
                "&field3=" + String(f3);

  int code = http.POST(body);
  String resp = http.getString();
  Serial.printf("ThingSpeak %d body=%s resp=%s\n", code, body.c_str(), resp.c_str());
  http.end();
}

void maybePostTS(bool force=false) {
  unsigned long now = millis();
  if (force || (now - lastTsPost) > TS_MIN_INTERVAL) {
    lastTsPost = now;
    postThingSpeakSnapshot();
  }
}

// =====================
// Telemetria
// =====================
void publishTelemetry(bool present) {
  StaticJsonDocument<256> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["sector"]   = SECTOR;
  doc["vagaId"]   = VAGA_ID;
  doc["status"]   = present ? "ocupada" : "livre";
  doc["present"]  = present;
  doc["ts"]       = iso8601Utc();

  String msg;
  serializeJson(doc, msg);

  mqtt.publish(topicTelemetry().c_str(), msg.c_str(), false);
  Serial.printf("PUB %s %s\n", topicTelemetry().c_str(), msg.c_str());

  // Exemplo opcional de POST HTTP direto para .NET:
  // HTTPClient http; WiFiClient wc;
  // http.begin(wc, API_URL); http.addHeader("Content-Type","application/json");
  // int code = http.POST(msg); String resp = http.getString(); http.end();
}

// =====================
// Setup/Loop
// =====================
void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  connectWiFi();
  connectMQTT();
}

void loop() {
  ensureWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  bool present = (digitalRead(SENSOR_PIN) == LOW);
  digitalWrite(LED_PIN, present ? HIGH : LOW);

  if (String(SECTOR) == "analise")       s_analise = present ? 1 : 0;
  else if (String(SECTOR) == "manutencao") s_manut   = present ? 1 : 0;
  else if (String(SECTOR) == "liberadas")  s_liber   = present ? 1 : 0;

  if (present != lastPresence) {
    lastPresence = present;
    publishTelemetry(present);
    maybePostTS(true);
  }

  maybePostTS(false);
  delay(200);
}
