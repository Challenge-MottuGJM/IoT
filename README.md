# 📡 Projeto IoT: Monitoramento de Motos por Setor (ESP32 + MQTT + Wokwi)

Este projeto simula um sistema de monitoramento de motos em um pátio com diferentes setores utilizando ESP32, sensores de presença, MQTT e visualização via Node-RED Dashboard.

## 🧠 Objetivo

Identificar em qual setor uma moto está localizada (Análise, Manutenção ou Liberadas) e enviar automaticamente essa informação via MQTT, com visualização em tempo real no dashboard do Node-RED.

---

## 🛠 Tecnologias Utilizadas

- ESP32 (simulado via [Wokwi](https://wokwi.com))
- MQTT (Mosquitto Broker)
- Wi-Fi (rede simulada Wokwi)
- Node-RED + Dashboard
- Visual Studio Code + PlatformIO

---

## 📂 Estrutura do Projeto

```
.
├── main.cpp             # Código principal (setor configurável)
├── diagram.json         # Circuito Wokwi (ESP32, botão e LED)
├── wokwi.toml           # Arquivo de configuração da simulação
├── platformio.ini       # Configuração do ambiente PlatformIO
```

---

## ▶️ Como Executar no VS Code (usando Wokwi CLI)

### 1. Compile o projeto

No terminal do VS Code:

```bash
pio run
```

> Isso gera o firmware `.elf` e `.hex` na pasta `.pio/build/esp32dev`.

---

### 2. Rode a simulação com Wokwi CLI

```bash
wokwi
```

---

### 3. Interaja com o circuito

- Pressione o botão → simula a entrada da moto no setor
- O LED acende enquanto a moto estiver presente
- A mensagem MQTT será publicada automaticamente

---

## 📡 Tópicos MQTT utilizados

- `patio/moto/analise`
- `patio/moto/manutencao`
- `patio/moto/liberadas`

> A mensagem enviada será no formato:

```json
{
  "motoId": "MOTO_LIB",
  "presente": true,
  "status": "entrou"
}
```

---

## 🖥 Visualização no Node-RED

1. Importe o fluxo `nodered_mqtt_dashboard.json` no Node-RED
2. Configure o broker para `localhost:1883`
3. Acesse: `http://localhost:1880/ui`

---

## 🔧 Configurando o Setor

No arquivo `main.cpp`, edite as seguintes linhas para definir o setor:

```cpp
const char* setor = "manutencao";
const char* motoId = "MOTO_MAN";
```

Use:
- `"analise"` e `"MOTO_ANA"` para setor de análise
- `"manutencao"` e `"MOTO_MAN"` para manutenção
- `"liberadas"` e `"MOTO_LIB"` para motos prontas

---

## ✅ Requisitos

- PlatformIO instalado no VS Code
- Wokwi CLI instalado e configurado (com token)
- Mosquitto MQTT Broker em execução
- Node-RED rodando com dashboard habilitado

---

## 📄 Licença

Projeto acadêmico - livre para uso educacional.