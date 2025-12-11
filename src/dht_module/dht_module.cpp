// dht_module.cpp
#include "dht_module.h"
#include "topics.h"

#include <DHT.h>
#include <PubSubClient.h>

extern PubSubClient client;
extern bool allowDHT;   // biến global định nghĩa ở main.cpp

static DHT dht(DHTPIN, DHTTYPE);
static unsigned long lastDHT = 0;
static const unsigned long DHT_INTERVAL = 2000;

void dht_setup() {
  dht.begin();
}

void dht_loop() {
  if (millis() - lastDHT < DHT_INTERVAL) return;
  lastDHT = millis();

  if (allowDHT) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      String tStr = String(t, 2);
      String hStr = String(h, 2);

      client.publish(TOPIC_DHT_TEMPERATURE, tStr.c_str());
      client.publish(TOPIC_DHT_HUMIDITY,    hStr.c_str());
      client.publish(TOPIC_DHT_STATUS,
                     (t >= 35 || h >= 60) ? "ERROR" : "OK");
    }
  } else {
    client.publish(TOPIC_DHT_TEMPERATURE, "0");
    client.publish(TOPIC_DHT_HUMIDITY,    "0");
    client.publish(TOPIC_DHT_STATUS,      "OFF");
  }
}
