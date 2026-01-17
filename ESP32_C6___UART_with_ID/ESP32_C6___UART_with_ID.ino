#include "SD_Card.h"
#include "Display_ST7789.h"
#include "LCD_Image.h"

char serialCode[5];
char foundFile[64];
const int DEVICE_ID = 2;

void setup() {
  Serial.begin(115200);
  Flash_test();
  LCD_Init();
  SD_Init();
  delay(3000);
  Serial.print("SUCCES JOSS");
  pinMode(BOOT_KEY_PIN, INPUT);

  Serial1.begin(115200, SERIAL_8N1, 18, 9);
}

void loop() {
  if (Serial1.available() > 0) {
    String s = Serial1.readString();
    s.trim();
    Serial.println("Frame diterima: " + s);

    int idxID = s.indexOf("id");
    int idxIMG = s.indexOf("img");

    if (idxID == -1 || idxIMG == -1) {
      Serial.println("Format tidak valid");
      return;
    }

    int receivedID = s.substring(idxID + 2, idxIMG).toInt();
    String imgName = s.substring(idxIMG + 3);

    if (receivedID != DEVICE_ID) {
      Serial.println("ID tidak cocok, abaikan");
      return;
    }

    String fullPath = "/" + imgName + ".png";
    Serial.println("Load image: " + fullPath);
    Serial1.println("Load image: " + fullPath);
    Show_Image(fullPath.c_str());
  }
  delay(5);
}