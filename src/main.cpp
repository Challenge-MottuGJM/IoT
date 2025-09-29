#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// =====================
// CONFIGURAÇÕES GERAIS
// =====================
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
const char* broker   = "test.mosquitto.org";
const uint16_t brokerPort = 1883;

// =====================
// IDENTIFICAÇÃO POR SETOR
// =====================

// === SETOR DE ANÁLISE ===
const char* setor  = "analise";   // "analise" | "manutencao" | "liberadas"
const char* motoId = "MOTO_ANA";

// === SETOR DE MANUTENÇÃO ===
// const char* setor  = "manutencao";
// const char* motoId = "MOTO_MAN";

// === SETOR DE LIBERADAS ===
// const char* setor  = "liberadas";
// const char* motoId = "MOTO_LIB";

// =====================
// THINGSPEAK (HTTP REST)
// =====================
const char* TS_URL       = "http://api.thingspeak.com/update";
const char* TS_WRITE_KEY = "SUA_WRITE_KEY";
const unsigned long TS_MIN_INTERVAL = 16000; // > 15s
unsigned long lastTsPost = 0;

// =====================
// HARDWARE
// =====================
const int sensorPin = 2;   // Botão (simula presença da moto)
const int ledPin    = 13;  // LED acende quando presente
bool lastState = false;

// Snapshot do pátio (3 setores)
int s_analise = 0;
int s_manut   = 0;
int s_liber   = 0;

// =====================
// REDES
// =====================
WiFiClient espClient;
PubSubClient client(espClient);

// ---------------------
// Funções de rede
// ---------------------
void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado!");
}

void conectarMQTT() {
  client.setServer(broker, brokerPort);
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falha, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// ---------------------
// ThingSpeak via HTTP
// ---------------------
void postThingSpeakSnapshot() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  WiFiClient wclient;
  http.begin(wclient, TS_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Mapeamento: field1=analise, field2=manutencao, field3=liberadas
  String body = "api_key=" + String(TS_WRITE_KEY) +
                "&field1=" + String(s_analise) +
                "&field2=" + String(s_manut) +
                "&field3=" + String(s_liber);

  int code = http.POST(body);
  Serial.printf("ThingSpeak POST %d body=%s\n", code, body.c_str());
  http.end();
}

void maybePublishThingSpeak(bool force = false) {
  unsigned long now = millis();
  if (force || now - lastTsPost > TS_MIN_INTERVAL) {
    lastTsPost = now;
    postThingSpeakSnapshot();
  }
}

// =====================
// SETUP/LOOP
// =====================
void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  conectarWiFi();
  client.setServer(broker, brokerPort);
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();

  bool presente = digitalRead(sensorPin) == LOW;
  digitalWrite(ledPin, presente ? HIGH : LOW);

  // Atualiza o snapshot conforme o setor deste dispositivo
  if (String(setor) == "analise")        s_analise = presente ? 1 : 0;
  else if (String(setor) == "manutencao") s_manut   = presente ? 1 : 0;
  else if (String(setor) == "liberadas")  s_liber   = presente ? 1 : 0;

  // Publica eventos on-change (MQTT local) e agenda envio ao ThingSpeak
  if (presente != lastState) {
    lastState = presente;
    String topic = "patio/moto/" + String(setor);
    String mensagem = "{";
    mensagem += "\"motoId\":\"" + String(motoId) + "\",";
    mensagem += "\"presente\":" + String(presente ? "true" : "false") + ",";
    mensagem += "\"status\":\"" + String(presente ? "entrou" : "saiu") + "\"";
    mensagem += "}";
    client.publish(topic.c_str(), mensagem.c_str());
    Serial.print("Publicado em ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(mensagem);

    // Dispara ThingSpeak respeitando rate limit (envia já ou no próximo heartbeat)
    maybePublishThingSpeak();
  }

  // Heartbeat periódico para atualizar o dashboard (>= 16s)
  maybePublishThingSpeak(false);

  delay(200);
}
