// led_module.cpp
#include "led_module.h"
#include "topics.h"

#include <PubSubClient.h>
extern PubSubClient client;    // dùng client từ mqtt_connect.h (đã define)

// ================== BIẾN NỘI BỘ ==================
static unsigned long lastLED = 0;
static const unsigned long LED_INTERVAL = 250;

static bool buzzerActive = false;
static uint8_t buzzerStep = 0;
static unsigned long buzzerLastTime = 0;

void setup_led() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void buzzer_start() {
  if (!buzzerActive) {
    buzzerActive = true;
    buzzerStep = 0;
  }
}

// Buzzer non-blocking (giữ nguyên logic cũ)
static void handleBuzzer() {
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

void led_loop(bool gpsOK) {
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

  // Luôn xử lý buzzer trong mỗi vòng lặp
  handleBuzzer();
}
