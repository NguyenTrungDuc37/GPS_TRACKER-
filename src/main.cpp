#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <MFRC522.h>
#include <SPI.h>

#include "wifi_connect.h"
#include "mqtt_connect.h"

#include "topics.h"
#include "led_module/led_module.h"
#include "gps_module/gps_module.h"
#include "rfid_module/rfid_module.h"
#include "dht_module/dht_module.h"


// ================== ĐỊNH NGHĨA TOPIC (dùng chung) ==================
const char* TOPIC_LED_STATUS        = "esp32/led/status";
const char* TOPIC_GPS_DATA          = "esp32/gps/data";
const char* TOPIC_RFID_UID          = "esp32/rfid/uid";
const char* TOPIC_DHT_TEMPERATURE   = "esp32/dht11/temperature";
const char* TOPIC_DHT_HUMIDITY      = "esp32/dht11/humidity";
const char* TOPIC_DHT_STATUS        = "esp32/dht11/status";

// ================== FLAG DHT (điều khiển qua MQTT) ==================
bool allowDHT = false;   // mqtt_connect.h có 'extern bool allowDHT;'

// ================== SETUP ==================
void setup() {
  Serial.begin(9600);

  dht_setup();
  setup_led();

  setup_wifi();
  mqtt_setup();

  gps_setup();
  rfid_setup();

  Serial.println("System ready!");
}

// ================== LOOP ==================
void loop() {
  // Xử lý WiFi & MQTT non-blocking
  wifi_handle();
  mqtt_handle();

  // Các module chức năng
  gps_loop();                // đọc + publish GPS
  rfid_loop();               // đọc + publish RFID
  dht_loop();                // đọc + publish DHT
  led_loop(gps_isValid());   // chớp LED + buzzer theo trạng thái GPS
}
