#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <WiFi.h>
#include "secrets/wifi.h"   // namespace WiFiSecrets

const unsigned long WIFI_RETRY_INTERVAL = 5000;

static bool wifiConnectingStarted = false;
static unsigned long lastWifiAttempt = 0;
static wl_status_t prevWifiStatus = WL_IDLE_STATUS;

void setup_wifi() {
  Serial.println("Starting WiFi (non-blocking)...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiSecrets::ssid, WiFiSecrets::passwd);
  wifiConnectingStarted = true;
  lastWifiAttempt = millis();
}

void wifi_handle() {
  wl_status_t st = WiFi.status();
  unsigned long now = millis();

  // Log khi trạng thái thay đổi
  if (st != prevWifiStatus) {
    prevWifiStatus = st;
    if (st == WL_CONNECTED) {
      Serial.print("WiFi CONNECTED! IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.print("WiFi status: ");
      Serial.println((int)st);
    }
  }

  if (st == WL_CONNECTED) return;  // đã kết nối thì thôi

  // Nếu chưa từng hoặc quá thời gian thì thử nối lại
  if (!wifiConnectingStarted || now - lastWifiAttempt >= WIFI_RETRY_INTERVAL) {
    Serial.println("WiFi (re)connecting...");
    WiFi.disconnect();
    WiFi.begin(WiFiSecrets::ssid, WiFiSecrets::passwd);
    wifiConnectingStarted = true;
    lastWifiAttempt = now;
  }
}

#endif
