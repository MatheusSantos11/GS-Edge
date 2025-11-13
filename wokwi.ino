#include <WiFi.h>
#include <PubSubClient.h>

#define PIN_LDR 34
#define PIN_PIR 27
#define PIN_LED 2

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";


WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32_Wokwi")) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PIR, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int ldrValue = analogRead(PIN_LDR);
  int pirValue = digitalRead(PIN_PIR);

  float luminosidade = map(ldrValue, 0, 4095, 0, 100);

  if (pirValue == 1 && luminosidade < 50) {
    digitalWrite(PIN_LED, HIGH);
    client.publish("/sala/luz", "1");
  } else {
    digitalWrite(PIN_LED, LOW);
    client.publish("/sala/luz", "0");
  }

  char msgLdr[10];
  char msgPir[10];
  dtostrf(luminosidade, 4, 2, msgLdr);
  sprintf(msgPir, "%d", pirValue);

  client.publish("/sala/luminosidade", msgLdr);
  client.publish("/sala/presenca", msgPir);

  Serial.print("Luminosidade: ");
  Serial.print(luminosidade);
  Serial.print(" | Presença: ");
  Serial.println(pirValue);

  delay(1000);
}
