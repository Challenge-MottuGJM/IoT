#include <WiFi.h>
#include <PubSubClient.h>

// =====================
// CONFIGURAÇÕES GERAIS
// =====================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* broker = "";

// =====================
// IDENTIFICAÇÃO POR SETOR
// =====================

// === SETOR DE ANÁLISE ===
const char* setor = "analise";
const char* motoId = "MOTO_ANA";

// === SETOR DE MANUTENÇÃO ===
// const char* setor = "manutencao";
// const char* motoId = "MOTO_MAN";

// === SETOR DE LIBERADAS ===
//const char* setor = "liberadas";
//const char* motoId = "MOTO_LIB";

// =====================

WiFiClient espClient;
PubSubClient client(espClient);

const int sensorPin = 2;   // Botão (simula presença da moto)
const int ledPin = 13;     // LED acende quando presente
bool lastState = false;

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

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  conectarWiFi();
  client.setServer(broker, 1883);
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();

  bool presente = digitalRead(sensorPin) == LOW;

  digitalWrite(ledPin, presente ? HIGH : LOW);

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
  }

  delay(500);
}
