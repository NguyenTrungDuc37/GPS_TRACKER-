// rfid_module.cpp
#include "rfid_module.h"
#include "topics.h"
#include "../led_module/led_module.h"

#include <MFRC522.h>
#include <SPI.h>
#include <PubSubClient.h>

extern PubSubClient client;

static MFRC522 rfid(RFID_SS, RFID_RST);

void rfid_setup() {
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();
  Serial.println("RFID Ready!");
}

void rfid_loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println(uid);

  // Bật buzzer khi quẹt thẻ
  buzzer_start();

  // Publish UID lên MQTT
  client.publish(TOPIC_RFID_UID, uid.c_str());

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
