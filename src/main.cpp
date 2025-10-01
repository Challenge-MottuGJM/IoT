#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// =====================
// PARAMETROS POR AMBIENTE (definidos no platformio.ini)
// =====================
#ifndef DEVICE_ID
  #define DEVICE_ID "espA"
#endif

#ifndef SECTOR
  #define SECTOR "analise"   // "analise" | "manutencao" | "liberadas"
#endif

#ifndef SENSOR_PIN
  #define SENSOR_PIN 2       // botão/presença
#endif

#ifndef LED_PIN
  #define LED_PIN 13         // LED indicador
#endif

// =====================
// REDE E BROKER
// =====================
const char* WIFI_SSID   = "Wokwi-GUEST";
const char* WIFI_PASS   = "";
const char* MQTT_HOST   = "test.mosquitto.org";
const uint16_t MQTT_PORT = 1883;

// =====================
// THINGSPEAK
// =====================
const char* TS_URL       = "http://api.thingspeak.com/update";
const char* TS_WRITE_KEY = "BC0PPUUCZGJLUR6O";
const unsigned long TS_MIN_INTERVAL = 16000; // >15s
unsigned long lastTsPost = 0;

// =====================
// ESTADO LOCAL
// =====================
bool lastPresence = false;

// Cada device publica apenas seu campo (1..3) para evitar colisões
int s_analise = 0;
int s_manut   = 0;
int s_liber   = 0;

// =====================
// MQTT
// =====================
WiFiClient espClient;
PubSubClient mqtt(espClient);

String topicTelemetry() { return String("motttu/telemetry/") + DEVICE_ID; }
String topicCmd()       { return String("motttu/actuators/") + DEVICE_ID + "/set"; }
String topicStatus()    { return String("motttu/status/") + DEVICE_ID; }

// ---------------------
// Conexões
// ---------------------
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" OK");
}

void onMqttMessage(char* topic, byte* payload, unsigned int len) {
  String t = topic;
  String body; body.reserve(len);
  for (unsigned int i=0;i<len;i++) body += (char)payload[i];
  Serial.printf("CMD %s %s\n", t.c_str(), body.c_str());

  // Comandos simples para demonstração
  if (body.indexOf("\"led\":1")>=0) digitalWrite(LED_PIN, HIGH);
  if (body.indexOf("\"led\":0")>=0) digitalWrite(LED_PIN, LOW);
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String cid = String("ESP32-") + DEVICE_ID + "-" + String((uint32_t)esp_random(), HEX);
    // LWT para detectar "desaparecida"
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

// ---------------------
// ThingSpeak
// ---------------------
void postThingSpeakSnapshot() {
  if (WiFi.status() != WL_CONNECTED) return;

  int f1=0, f2=0, f3=0;
  if (String(SECTOR) == "analise") f1 = s_analise;
  else if (String(SECTOR) == "manutencao") f2 = s_manut;
  else if (String(SECTOR) == "liberadas") f3 = s_liber;

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

// ---------------------
// Setup/Loop
// ---------------------
void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  bool present = (digitalRead(SENSOR_PIN) == LOW);
  digitalWrite(LED_PIN, present ? HIGH : LOW);

  // Atualiza snapshot local para o setor deste device
  if (String(SECTOR) == "analise")        s_analise = present ? 1 : 0;
  else if (String(SECTOR) == "manutencao") s_manut   = present ? 1 : 0;
  else if (String(SECTOR) == "liberadas")  s_liber   = present ? 1 : 0;

  // Evento on-change
  if (present != lastPresence) {
    lastPresence = present;
    String msg = String("{\"deviceId\":\"") + DEVICE_ID + "\","
                + "\"sector\":\"" + SECTOR + "\","
                + "\"present\":" + (present ? "true":"false") + ","
                + "\"ts\":" + String((uint32_t)(millis()/1000)) + "}";
    mqtt.publish(topicTelemetry().c_str(), msg.c_str(), false);
    Serial.printf("PUB %s %s\n", topicTelemetry().c_str(), msg.c_str());
    maybePostTS(true);
  }

  // Heartbeat TS (respeita rate limit)
  maybePostTS(false);

  delay(200);
}
