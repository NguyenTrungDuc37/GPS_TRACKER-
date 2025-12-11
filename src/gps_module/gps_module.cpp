// gps_module.cpp
#include "gps_module.h"
#include "topics.h"

#include <TinyGPS++.h>
#include <PubSubClient.h>

extern PubSubClient client;

static TinyGPSPlus gps;
static unsigned long lastGPS = 0;
static const unsigned long GPS_INTERVAL = 1000;

void gps_setup() {
  Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS Started");
}

bool gps_isValid() {
  return gps.location.isValid();
}

void gps_loop() {
  // Nạp dữ liệu từ module GPS
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // Gửi MQTT theo chu kỳ
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
  int hour   = gps.time.hour() + 7;  // GMT+7
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
