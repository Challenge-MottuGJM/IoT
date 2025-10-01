# 📡 Projeto IoT: Monitoramento por Setor (MQTT + Coletor Node + ThingSpeak)

Este projeto simula um pátio com três setores (Análise, Manutenção, Liberadas), publica eventos via MQTT e registra tudo de forma persistente (SQLite/CSV) através de um coletor Node.js, além de atualizar um canal ThingSpeak para visualização em tempo real.

## 🧠 Objetivo

- Identificar em qual setor uma moto está localizada (Análise, Manutenção ou Liberadas)
  
- Simular presença por setor e publicar eventos no broker MQTT.

- Coletar e persistir os eventos em banco local (SQLite) e arquivo CSV.

- Enviar o snapshot dos três setores para o ThingSpeak a cada ~20 s e visualizar nos fields do canal.



---

## 🛠 Tecnologias Utilizadas

- ESP32 (simulado via [Wokwi](https://wokwi.com))
- MQTT (Mosquitto Broker)
- Node.js (coletor com mqtt + better‑sqlite3 + node-fetch)
- ThingSpeak (canal com fields por setor)
- Visual Studio Code + PlatformIO

---

## 📂 Estrutura do Projeto

```
IOT/
├── src/main.cpp                # Firmware ESP32 (opcional para uso real)
├── platformio.ini              # Ambientes espA/espB/espC (opcional)
├── diagram.json / wokwi.toml   # Simulação Wokwi (opcional)
└── iot-collector/              # Coletor e simulador (Node.js)
    ├── collector.js            # Assina MQTT, salva em SQLite/CSV e envia ao ThingSpeak [web:771][web:866][web:893]
    ├── simulator.js            # Publica eventos simulados para espA/espB/espC [web:771]
    ├── iot_events.db           # Banco SQLite (gerado em runtime) [web:866]
    └── events.csv              # Log CSV (gerado em runtime) [web:866]
```

---

## ▶️ Como Executar no VS Code (sem hardware)

### 1. Preparar o coletor

No diretório IOT/iot-collector:

```bash
- npm init -y
- npm i mqtt better-sqlite3 node-fetch
```

No package.json, inclua "type": "module" para permitir import ESM.
Em collector.js:

- Configure a Write API Key do seu canal ThingSpeak (TS_WRITE_KEY) e confirme o mapeamento field1=analise, field2=manutencao, field3=liberadas.
  
Rodar o coletor:
- node collector.js → deve exibir “MQTT conectado”.
---

### 2. Rodar o simulador de eventos

- Em outro terminal na mesma pasta:
```bash
node simulator.js
```
> publica mensagens alternando entre espA/espB/espC a cada ~7 s.
O collector mostrará “EVENTO …” e criará/atualizará iot_events.db e events.csv.
---

### 3. Conferir persistência

CSV crescendo a cada evento.
Consultas rápidas (via script ou CLI):
- Contagem por setor/estado e última leitura.

---

### 4. Visualização no ThingSpeak

O collector envia o snapshot a cada 20 s para o endpoint /update com api_key + field1..3.
Em cada Chart do canal:
- Type: Step (degrau)
- Y‑Axis Min = 0, Y‑Axis Max = 1
- Title curto por setor
- Active Refresh (Dynamic) se desejar autoupdate

## 📡 Tópicos MQTT utilizados

- `Publicação de telemetria: motttu/telemetry/espA | espB | espC`
- `patio/moto/manutencao`
- `patio/moto/liberadas`

> Payload de exemplo:

```json
{
"deviceId":"espA",
"sector":"analise",
"present":true,
"ts": 1710000000
}
```

---

## Mapeamento de Fields no ThingSpeak

- field1 → Análise
- field2 → Manutenção
- field3 → Liberadas
Atualização mínima respeitada pelo coletor: 20 s (recomendado para plano free).
---

## ✅ Requisitos

- PlatformIO instalado no VS Code
- Node.js 18+ instalado
- Wokwi CLI instalado e configurado (com token)
- Acesso ao broker MQTT público (test.mosquitto.org)
- Canal ThingSpeak criado com 3 fields e Write API Key configurada no collector

---
### 🧑‍🤝‍🧑 Integrantes do Projeto

- **Gustavo de Aguiar Lima Silva** - RM: 557707  
- **Julio Cesar Conceição Rodrigues** - RM: 557298  
- **Matheus de Freitas Silva** - RM: 552602

---

## 📄 Licença

Projeto acadêmico - livre para uso educacional.
