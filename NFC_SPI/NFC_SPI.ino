#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>

// PIN SPI PN532
#define PN532_SCK  13
#define PN532_MOSI 11
#define PN532_MISO 12
#define PN532_SS   10

// Gunakan driver SPI
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

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
          asciiDump += blockToAscii(data);
        }
      }
      completed = true;
    } else {
      completed = false;
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);

  led_rgb.begin();
  led_rgb.setBrightness(100);

  // --- Cek PN532 ---
  Serial.println("Mencari PN532...");
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();

  if (!version) {
    Serial.println("PN532 TIDAK TERDETEKSI!");
    Serial.println("Periksa wiring / mode SPI!");
    
    led_rgb.setPixelColor(0, led_rgb.Color(255, 0, 0)); // merah = error
    led_rgb.show();

    while (1) {
      delay(100); // berhenti total
    }
  }

  // --- Jika terdeteksi ---
  Serial.print("PN532 terdeteksi! Firmware: ");
  Serial.println(version, HEX);

  led_rgb.setPixelColor(0, led_rgb.Color(0, 255, 0)); // hijau = OK
  led_rgb.show();
  delay(500);

  nfc.SAMConfig();
}

void loop() {
  led_rgb.clear();
  led_rgb.show();

  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    led_rgb.setPixelColor(0, uint32_t(led_rgb.Color(255, 0, 0)));
    led_rgb.show();
    delay(500);

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
