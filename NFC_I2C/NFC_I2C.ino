#include <Wire.h>
#include <Adafruit_PN532.h>

#define SDA_PIN 8
#define SCL_PIN 9
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void readAllSectors(uint8_t *uid, uint8_t uidLength) {
    Serial.println("Dumping all sectors...");
    for (uint8_t sector = 0; sector < 16; sector++) {
        uint8_t blockStart = sector * 4;
        if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockStart, 0, keya)) {
            for (uint8_t block = 0; block < 4; block++) {
                uint8_t data[16];
                if (nfc.mifareclassic_ReadDataBlock(blockStart + block, data)) {
                    Serial.print("Block ");
                    Serial.print(blockStart + block);
                    Serial.print(": ");
                    for (uint8_t i = 0; i < 16; i++) {
                        Serial.print(data[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();
                }
            }
        } else {
            Serial.print("Failed sector ");
            Serial.println(sector);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing PN532...");
    nfc.begin();

    uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
        Serial.println("Didn't find PN532 board");
        while (1);
    }

    nfc.SAMConfig();
    Serial.println("PN532 initialized!");
}

void loop() {
    Serial.println("Waiting for NFC tag...");
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success) {
        Serial.print("Found NFC tag with UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        readAllSectors(uid, uidLength);
    }

    delay(1000);
}
