#ifndef MQTT_CONNECT_H
#define MQTT_CONNECT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets/mqtt.h"   // namespace EMQX

// biến allowDHT được định nghĩa ở main.cpp
extern bool allowDHT;

// ====== ĐỐI TƯỢNG MQTT ======
WiFiClientSecure espClient;
PubSubClient client(espClient);

const unsigned long MQTT_RECONNECT_INTERVAL = 3000;
static unsigned long lastMqttAttempt = 0;

// ====== CALLBACK ======
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (String(topic) == "esp32/switch") {
    allowDHT = (msg == "true");
    client.publish("esp32/switch/ack", allowDHT ? "ON" : "OFF");
  }
}

// ====== SETUP MQTT ======
void mqtt_setup() {
  espClient.setInsecure();
  client.setServer(EMQX::mqtt_server, EMQX::port);
  client.setCallback(mqttCallback);
}

// ====== HANDLE MQTT (non-blocking) ======
void mqtt_handle() {
  // Chưa có WiFi thì không cố nối MQTT
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastMqttAttempt >= MQTT_RECONNECT_INTERVAL) {
      lastMqttAttempt = now;

      String id = "ESP32_" + String(random(0xffff), HEX);
      Serial.print("MQTT connecting... ");

      if (client.connect(id.c_str(), EMQX::mqtt_username, EMQX::mqtt_password)) {
        Serial.println("OK!");
        client.subscribe("esp32/switch");

        client.publish("esp32/status/mqtt", "CONNECTED MQTT_BROKER");
        client.publish("esp32/status", "ONLINE");
        client.publish("esp32/status/wifi",
                       WiFi.status() == WL_CONNECTED ? "CONNECTED WIFI" : "DISCONNECTED WIFI");
      } else {
        Serial.print("Failed, rc=");
        Serial.println(client.state());
      }
    }
  } else {
    // đã connect thì phải loop thường xuyên
    client.loop();
  }
}

#endif
