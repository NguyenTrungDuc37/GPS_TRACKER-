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

// ================== MQTT TOPICS ==================
const char* TOPIC_LED_STATUS        = "esp32/led/status";
const char* TOPIC_GPS_DATA          = "esp32/gps/data";
const char* TOPIC_RFID_UID          = "esp32/rfid/uid";
const char* TOPIC_DHT_TEMPERATURE   = "esp32/dht11/temperature";
const char* TOPIC_DHT_HUMIDITY      = "esp32/dht11/humidity";
const char* TOPIC_DHT_STATUS        = "esp32/dht11/status";

// ================== FLAGS ==================
bool allowDHT = false;

// ================== OBJECTS ==================
TinyGPSPlus gps;
DHT dht(DHTPIN, DHTTYPE);
MFRC522 rfid(RFID_SS, RFID_RST);

// ================== TIMERS ==================
unsigned long lastGPS = 0;
unsigned long lastDHT = 0;
unsigned long lastLED = 0;

// ================== INTERVALS ==================
const unsigned long GPS_INTERVAL = 1000;
const unsigned long DHT_INTERVAL = 2000;
const unsigned long LED_INTERVAL = 250;

// ================== BUZZER STATE (non-blocking) ==================
bool buzzerActive = false;
uint8_t buzzerStep = 0;
unsigned long buzzerLastTime = 0;

void handleBuzzer() {
  if (!buzzerActive) return;

  unsigned long now = millis();

  switch (buzzerStep) {
    case 0:
      digitalWrite(BUZZER, HIGH);
      buzzerLastTime = now;
      buzzerStep = 1;
      break;

    case 1:
      if (now - buzzerLastTime >= 80) {
        digitalWrite(BUZZER, LOW);
        buzzerLastTime = now;
        buzzerStep = 2;
      }
      break;

    case 2:
      if (now - buzzerLastTime >= 60) {
        digitalWrite(BUZZER, HIGH);
        buzzerLastTime = now;
        buzzerStep = 3;
      }
      break;

    case 3:
      if (now - buzzerLastTime >= 80) {
        digitalWrite(BUZZER, LOW);
        buzzerLastTime = now;
        buzzerStep = 4;
      }
      break;

    case 4:
      if (now - buzzerLastTime >= 60) {
        buzzerActive = false;
        buzzerStep = 0;
      }
      break;
  }
}

// ================== LED ==================
void setup_led() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void setLED(bool gpsOK) {
  static bool st = false;

  if (millis() - lastLED >= LED_INTERVAL) {
    lastLED = millis();
    st = !st;

    if (gpsOK) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, st);
      client.publish(TOPIC_LED_STATUS, st ? "GREEN_ON" : "GREEN_OFF");
    } else {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, st);
      client.publish(TOPIC_LED_STATUS, st ? "RED_ON" : "RED_OFF");
    }
  }
}

// ================== GPS ==================
void setup_gps() {
  Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS Started");
}

void publishGPS() {
  if (millis() - lastGPS < GPS_INTERVAL) return;
  lastGPS = millis();

  char payload[256];

  double lat   = gps.location.isValid()    ? gps.location.lat()       : 0.0;
  double lng   = gps.location.isValid()    ? gps.location.lng()       : 0.0;
  double speed = gps.speed.isValid()       ? gps.speed.kmph()         : 0.0;
  double alt   = gps.altitude.isValid()    ? gps.altitude.meters()    : 0.0;
  int    sats  = gps.satellites.isValid()  ? gps.satellites.value()   : 0;
  double hdop  = gps.hdop.isValid()        ? gps.hdop.hdop()          : 0.0;

  int year   = gps.date.year();
  int month  = gps.date.month();
  int day    = gps.date.day();
  int hour   = gps.time.hour() + 7;
  int minute = gps.time.minute();
  int second = gps.time.second();
  if (hour >= 24) hour -= 24;

  char timeStr[25];
  snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
           year, month, day, hour, minute, second);

  snprintf(payload, sizeof(payload),
           "{\"lat\":%.6f,\"lng\":%.6f,\"speed\":%.2f,\"alt\":%.2f,"
           "\"sat\":%d,\"hdop\":%.2f,\"time\":\"%s\"}",
           lat, lng, speed, alt, sats, hdop, timeStr);

  client.publish(TOPIC_GPS_DATA, payload);
}

// ================== RFID ==================
void setup_rfid() {
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();
  Serial.println("RFID Ready!");
}

void readRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++)
    uid += String(rfid.uid.uidByte[i], HEX);

  uid.toUpperCase();
  Serial.println(uid);

  if (!buzzerActive) {
    buzzerActive = true;
    buzzerStep = 0;
  }

  client.publish(TOPIC_RFID_UID, uid.c_str());

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ================== SETUP ==================
void setup() {
  Serial.begin(9600);
  dht.begin();
  setup_led();
  setup_wifi();
  setup_gps();
  setup_rfid();

  mqtt_setup();

  Serial.println("System ready!");
}

// ================== LOOP ==================
void loop() {
  // Handle WiFi & MQTT non-blocking
  wifi_handle();
  mqtt_handle();

  // GPS data input
  while (Serial2.available() > 0) gps.encode(Serial2.read());

  setLED(gps.location.isValid());
  publishGPS();
  readRFID();
  handleBuzzer();   

  // ===== DHT (non-blocking) =====
  if (millis() - lastDHT >= DHT_INTERVAL) {
    lastDHT = millis();

    if (allowDHT) {
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      if (!isnan(h) && !isnan(t)) {
        client.publish(TOPIC_DHT_TEMPERATURE, String(t, 2).c_str());
        client.publish(TOPIC_DHT_HUMIDITY,    String(h, 2).c_str());
        client.publish(TOPIC_DHT_STATUS,
                       (t >= 35 || h >= 60) ? "ERROR" : "OK");
      }
    } else {
      client.publish(TOPIC_DHT_TEMPERATURE, "0");
      client.publish(TOPIC_DHT_HUMIDITY,    "0");
      client.publish(TOPIC_DHT_STATUS,      "OFF");
    }
  }
}
