// led_module.h
#pragma once
#include <Arduino.h>

void setup_led();
void led_loop(bool gpsOK);
void buzzer_start();   // gọi khi quẹt RFID
