#pragma once

// ===== Wi‑Fi =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

// ===== Identidade do dispositivo =====
#define DEVICE_ID "esp32-analise-01"     // altere por setor
#define SECTOR    "analise"              // "analise" | "manutencao" | "liberadas"
#define VAGA_ID   "A-01"

// ===== Pinos =====
#define SENSOR_PIN 4
#define LED_PIN    2

// ===== MQTT =====
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883

// ===== ThingSpeak (opcional) =====
#define TS_URL          "http://api.thingspeak.com/update"
#define TS_WRITE_KEY    "BC0PPUUCZGJLUR6O"
#define TS_MIN_INTERVAL 20000UL          // ≥15 s no plano gratuito

// ===== NTP/Tempo =====
#define NTP_SERVER         "pool.ntp.org"
#define TZ_OFFSET_SECONDS  0             // UTC; ajuste se precisar horário local
#define DST_OFFSET_SECONDS 0

// ===== .NET API (opcional) =====
// #define API_URL "http://seu-backend.local/api/eventos"
