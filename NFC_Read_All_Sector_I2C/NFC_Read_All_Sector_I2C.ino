#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>

#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);
Adafruit_NeoPixel led_rgb(1, 48, NEO_GRBW + NEO_KHZ800);

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

String asciiDump = "";
bool completed = false;

String blockToAscii(uint8_t *data) {
  String s = "";
  for (int i = 0; i < 16; i++) {
    if (data[i] >= 32 && data[i] <= 126)
      s += (char)data[i];
  }
  return s;
}

void readAllSectors(uint8_t *uid, uint8_t uidLength) {
  asciiDump = "";
  for (uint8_t sector = 0; sector < 16; sector++) {
    uint8_t blockStart = sector * 4;
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockStart, 0, keya)) {
      for (uint8_t block = 0; block < 3; block++) {
        uint8_t data[16];
        if (nfc.mifareclassic_ReadDataBlock(blockStart + block, data)) {
          // Serial.print("Block ");
          // Serial.print(blockStart + block);
          // Serial.print(": ");
          for (uint8_t i = 0; i < 16; i++) {
            // Serial.print(data[i], HEX);
            // Serial.print(" ");
          }
          // Serial.println();
          asciiDump += blockToAscii(data);
        }
      }
      completed = 1;
    } else {
      // Serial.print("Failed sector ");
      // Serial.println(sector);
      completed = 0;
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);
  nfc.begin();
  uint32_t version = nfc.getFirmwareVersion();
  if (!version) {
    // Serial.println("Didn't find PN532 board");
    while (1)
      ;
  }
  nfc.SAMConfig();
  // Serial.println("PN532 initialized!");

  led_rgb.begin();
  led_rgb.setBrightness(100);
}

void loop() {
  // Serial.println("Waiting for NFC tag...");
  led_rgb.clear();
  led_rgb.show();
  uint8_t success;
  uint8_t uid[7];
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    led_rgb.setPixelColor(0, uint32_t(led_rgb.Color(255, 0, 0)));
    led_rgb.show();
    delay(500);
    // Serial.print("Found NFC tag with UID: ");
    // for (uint8_t i = 0; i < uidLength; i++) {
    //   Serial.print(uid[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println();
    readAllSectors(uid, uidLength);
    if (completed) {
      led_rgb.setPixelColor(0, uint32_t(led_rgb.Color(0, 255, 0)));
      led_rgb.show();
      Serial.println(asciiDump);
      delay(1000);
    }
  }
  delay(100);
}
