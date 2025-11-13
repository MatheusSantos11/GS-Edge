# 🌱 Estação Inteligente de Economia de Energia

### Integrantes
| Nome                              | RM       |
|----------------------------------|-----------|
| Henrique de Oliveira Gomes       | RM566424  |
| Henrique Kolomyes Silveira       | RM563467  |
| Matheus Santos de Oliveira       | RM561982  |

---

## 📘 Descrição do Projeto

O projeto **“Estação Inteligente de Economia de Energia”** tem como objetivo demonstrar como a **IoT (Internet das Coisas)** pode ajudar na **redução do desperdício de energia elétrica** em ambientes de trabalho ou estudo.

O sistema é composto por:
- Um **ESP32**, que coleta dados de **luminosidade (LDR)** e **presença (PIR)**;
- Um **servidor Node-RED**, que exibe os dados em tempo real em uma **dashboard**;
- Um **script Python**, que recebe os dados via **MQTT**, gera gráficos e calcula estatísticas de economia energética.

---

## 🧩 Fluxo Geral

```
LDR + PIR → ESP32 → MQTT (broker.hivemq.com) → Node-RED Dashboard
                                             ↘
                                              Python (gráficos e cálculos)
```

---

## ⚙️ Tecnologias Utilizadas

| Componente | Descrição |
|-------------|------------|
| **ESP32 (Wokwi)** | Microcontrolador com Wi-Fi, responsável por ler sensores |
| **Sensor LDR** | Mede luminosidade do ambiente |
| **Sensor PIR** | Detecta presença humana |
| **MQTT (HiveMQ)** | Protocolo de comunicação leve usado para IoT |
| **Node-RED** | Plataforma de automação e visualização dos dados |
| **Python 3 + paho-mqtt + matplotlib + pandas** | Processamento e gráficos locais |
| **Broker público:** `broker.hivemq.com` | Usado para conectar os dispositivos |

---

## 🛰️ Comunicação MQTT

| Tópico | Direção | Descrição | Exemplo de mensagem |
|--------|----------|-----------|---------------------|
| `/sala/luminosidade` | ESP32 → Node-RED/Python | Envia a luminosidade lida pelo LDR | `82.4` |
| `/sala/presenca` | ESP32 → Node-RED/Python | Envia 1 (movimento) ou 0 (sem presença) | `1` |
| `/sala/economia` *(opcional)* | Python → Node-RED | Estimativa de energia economizada | `2.5` |

---

## 🔌 Código do ESP32 (Wokwi)

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqttServer = "broker.hivemq.com";
int port = 1883;

#define LDR_PIN 34
#define PIR_PIN 14

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("✅ WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32_Energia")) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 2s...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  setup_wifi();
  client.setServer(mqttServer, port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int ldrValue = analogRead(LDR_PIN);
  int pirValue = digitalRead(PIR_PIN);

  float lumPercent = (ldrValue / 4095.0) * 100.0;

  char msgLdr[10];
  char msgPir[10];
  dtostrf(lumPercent, 4, 2, msgLdr);
  sprintf(msgPir, "%d", pirValue);

  client.publish("/sala/luminosidade", msgLdr);
  client.publish("/sala/presenca", msgPir);

  Serial.print("Luminosidade: ");
  Serial.print(lumPercent);
  Serial.print(" | Presença: ");
  Serial.println(pirValue);

  delay(2000);
}
```

---

## 🐍 Código Python

```python
import paho.mqtt.client as mqtt
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC_LDR = "/sala/luminosidade"
TOPIC_PIR = "/sala/presenca"

dados = {"tempo": [], "luminosidade": [], "presenca": []}

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Conectado ao broker MQTT!")
        client.subscribe([(TOPIC_LDR, 0), (TOPIC_PIR, 0)])
    else:
        print(f"❌ Falha ao conectar, código {rc}")

def on_message(client, userdata, msg):
    global dados
    valor = float(msg.payload.decode())

    if msg.topic == TOPIC_LDR:
        dados["luminosidade"].append(valor)
        dados["presenca"].append(dados["presenca"][-1] if dados["presenca"] else 0)
    elif msg.topic == TOPIC_PIR:
        dados["presenca"].append(valor)
        dados["luminosidade"].append(dados["luminosidade"][-1] if dados["luminosidade"] else 0)

    dados["tempo"].append(datetime.now().strftime("%H:%M:%S"))
    print(f"📨 {msg.topic}: {valor}")

    if len(dados["tempo"]) > 20:
        for k in dados.keys():
            dados[k] = dados[k][-20:]

    df = pd.DataFrame(dados)
    plt.clf()
    plt.plot(df["tempo"], df["luminosidade"], label="Luminosidade (%)")
    plt.plot(df["tempo"], df["presenca"], label="Presença (1/0)")
    plt.xticks(rotation=45)
    plt.legend()
    plt.title("Monitoramento em tempo real - Energia Inteligente")
    plt.tight_layout()
    plt.pause(0.1)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("🔌 Conectando ao broker...")
client.connect(BROKER, PORT, 60)

plt.ion()
plt.show()

client.loop_forever()
```

---

## 🧱 Node-RED

O JSON completo do Node-RED (com gauges, textos e gráficos) está pronto para importação e utiliza os mesmos tópicos MQTT (`/sala/luminosidade`, `/sala/presenca`).
Importe-o pelo menu: **Import > Clipboard**.
```node
[
  {
    "id": "a1b2c3d4e5f6g7h8",
    "type": "tab",
    "label": "Energia Inteligente",
    "disabled": false,
    "info": ""
  },
  {
    "id": "mqtt_in_luminosidade",
    "type": "mqtt in",
    "z": "a1b2c3d4e5f6g7h8",
    "name": "Luminosidade",
    "topic": "/sala/luminosidade",
    "qos": "0",
    "datatype": "auto",
    "broker": "mqtt_broker_config",
    "nl": false,
    "rap": false,
    "rh": 0,
    "x": 160,
    "y": 120,
    "wires": [["luminosidade_ui", "luminosidade_chart", "python_out"]]
  },
  {
    "id": "mqtt_in_presenca",
    "type": "mqtt in",
    "z": "a1b2c3d4e5f6g7h8",
    "name": "Presença",
    "topic": "/sala/presenca",
    "qos": "0",
    "datatype": "auto",
    "broker": "mqtt_broker_config",
    "nl": false,
    "rap": false,
    "rh": 0,
    "x": 150,
    "y": 200,
    "wires": [["presenca_ui", "presenca_chart", "python_out"]]
  },
  {
    "id": "luminosidade_ui",
    "type": "ui_text",
    "z": "a1b2c3d4e5f6g7h8",
    "group": "grupo_dashboard",
    "order": 1,
    "width": 0,
    "height": 0,
    "name": "Luminosidade Atual",
    "label": "Luminosidade Atual",
    "format": "{{msg.payload}}%",
    "layout": "row-spread",
    "x": 460,
    "y": 120,
    "wires": []
  },
  {
    "id": "presenca_ui",
    "type": "ui_text",
    "z": "a1b2c3d4e5f6g7h8",
    "group": "grupo_dashboard",
    "order": 2,
    "width": 0,
    "height": 0,
    "name": "Presença Detectada",
    "label": "Presença Detectada",
    "format": "{{msg.payload == 1 ? 'Sim' : 'Não'}}",
    "layout": "row-spread",
    "x": 460,
    "y": 200,
    "wires": []
  },
  {
    "id": "luminosidade_chart",
    "type": "ui_chart",
    "z": "a1b2c3d4e5f6g7h8",
    "name": "Gráfico de Luminosidade",
    "group": "grupo_dashboard",
    "order": 3,
    "width": 0,
    "height": 0,
    "label": "Gráfico de Luminosidade (%)",
    "chartType": "line",
    "legend": "false",
    "xformat": "HH:mm:ss",
    "interpolate": "linear",
    "nodata": "",
    "dot": false,
    "ymin": "0",
    "ymax": "100",
    "removeOlder": "10",
    "removeOlderPoints": "",
    "removeOlderUnit": "60",
    "cutout": 0,
    "useOneColor": false,
    "colors": ["#00bfff"],
    "outputs": 1,
    "useDifferentColor": false,
    "x": 480,
    "y": 160,
    "wires": [[]]
  },
  {
    "id": "presenca_chart",
    "type": "ui_chart",
    "z": "a1b2c3d4e5f6g7h8",
    "name": "Gráfico de Presença",
    "group": "grupo_dashboard",
    "order": 4,
    "width": 0,
    "height": 0,
    "label": "Gráfico de Presença (0=Ausente, 1=Detectado)",
    "chartType": "line",
    "legend": "false",
    "xformat": "HH:mm:ss",
    "interpolate": "step",
    "ymin": "0",
    "ymax": "1",
    "removeOlder": "10",
    "removeOlderPoints": "",
    "removeOlderUnit": "60",
    "useOneColor": false,
    "colors": ["#ff6600"],
    "outputs": 1,
    "x": 480,
    "y": 240,
    "wires": [[]]
  },
  {
    "id": "python_out",
    "type": "mqtt out",
    "z": "a1b2c3d4e5f6g7h8",
    "name": "Python Output",
    "topic": "energia_inteligente/dados",
    "qos": "",
    "retain": "",
    "broker": "mqtt_broker_config",
    "x": 460,
    "y": 300,
    "wires": []
  },
  {
    "id": "mqtt_broker_config",
    "type": "mqtt-broker",
    "name": "Mosquitto Broker",
    "broker": "test.mosquitto.org",
    "port": "1883",
    "clientid": "",
    "usetls": false,
    "protocolVersion": "4",
    "keepalive": "60",
    "cleansession": true,
    "birthTopic": "",
    "birthQos": "0",
    "birthPayload": "",
    "closeTopic": "",
    "closePayload": "",
    "willTopic": "",
    "willQos": "0",
    "willPayload": ""
  },
  {
    "id": "grupo_dashboard",
    "type": "ui_group",
    "name": "Painel de Monitoramento",
    "tab": "tab_dashboard",
    "order": 1,
    "disp": true,
    "width": "6"
  },
  {
    "id": "tab_dashboard",
    "type": "ui_tab",
    "name": "Energia Inteligente",
    "icon": "dashboard",
    "order": 1
  }
]
```
---

## 🚀 Execução

1. No **Wokwi**, execute o código do ESP32.  
2. No **Node-RED**, importe o fluxo e abra a dashboard (`http://localhost:1880/ui`).  
3. Execute o Python com:
   ```bash
   python energia_inteligente.py
   ```

---

## 🧠 Conclusão

Este projeto demonstra a integração completa entre **IoT, automação e análise de dados**, promovendo o uso consciente de energia em ambientes controlados.
