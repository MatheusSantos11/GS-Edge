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
        print(f"📡 Inscrito nos tópicos:\n  - {TOPIC_LDR}\n  - {TOPIC_PIR}")
    else:
        print(f"❌ Falha ao conectar, código {rc}")


def on_message(client, userdata, msg):
    global dados
    try:
        valor = float(msg.payload.decode())
    except:
        valor = 0

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
    plt.plot(df["tempo"], df["luminosidade"], label="Luminosidade")
    plt.plot(df["tempo"], df["presenca"], label="Presença")
    plt.xticks(rotation=45)
    plt.legend()
    plt.title("Monitoramento em tempo real")
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
