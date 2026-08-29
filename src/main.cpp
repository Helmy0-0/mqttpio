#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
 
#define POT_PIN 34
#define WATER_PIN 35

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESPressoClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(POT_PIN, INPUT);
  pinMode(WATER_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish 
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    int potValue = analogRead(POT_PIN);
    float potVoltage = potValue * (3.3 / 4095.0);

    int waterRaw = analogRead(WATER_PIN);
    
    int waterPercent = map(waterRaw, 0, 2500, 0, 100);
    waterPercent = constrain(waterPercent, 0, 100);


    String potPayload = "{ \"pot_value\": " + String(potValue) +
                    ", \"pot_voltage\": " + String(potVoltage, 2) + " }";

    String waterPayload = "{ \"water_raw\": " + String(waterRaw) +
                     ", \"water_percent\": " + String(waterPercent) + " }";          

    String allPayload = "{ \"pot_value\": " + String(potValue) +
                     ", \"pot_voltage\": " + String(potVoltage, 2) +
                     ", \"water_raw\": " + String(waterRaw) +
                     ", \"water_percent\": " + String(waterPercent) + " }";

    Serial.print("Publish message: ");
    Serial.println(allPayload);

    client.publish("espresso/sensor/pot", potPayload.c_str());
    client.publish("espresso/sensor/water", waterPayload.c_str());
    client.publish("espresso/sensor", allPayload.c_str());
  }
}