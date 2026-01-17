#include "SD_Card.h"
#include "Display_ST7789.h"
#include "LCD_Image.h"

char serialCode[5];
char foundFile[64];
int DEVICE_ID = 0;
String onImage = "";
String offImage = "";
bool stby = true, stbyShown = false;

void setup() {
  Serial.begin(9600);
  Flash_test();
  LCD_Init();
  SD_Init();

  String imgStby = "/stby.png";
  Show_Image(imgStby.c_str());
  stbyShown = true;

  delay(3000);

  if (loadConfig()) {
    Serial.println("Config loaded sukses!");
  }

  Serial.println("SUCCES JOSS");
  pinMode(BOOT_KEY_PIN, INPUT);

  Serial1.begin(9600, SERIAL_8N1, 18, 9);
}

void loop() {
  if (stby && !stbyShown) {
    String imgStby = "/stby.png";
    Show_Image(imgStby.c_str());
    stbyShown = true;
  }

  if (Serial1.available() > 0) {
    String s = Serial1.readString();
    s.trim();
    Serial.println("Frame diterima: " + s);

    int idxID = s.indexOf("id");
    if (idxID == -1) {
      Serial.println("Format tidak valid");
      stby = true;
      return;
    }

    int receivedID = s.substring(idxID + 2).toInt();

    if (DEVICE_ID != 0 && receivedID != DEVICE_ID) {
      Serial.println("ID tidak cocok, abaikan");
      stby = true;
      return;
    }

    // if (s.endsWith("on")) {
    //   // Serial1.println("Load ON from ID " + String(DEVICE_ID) + ": " + onImage);
    //   stby = false;
    //   stbyShown = false;
    //   Show_Image(onImage.c_str());
    //   return;
    // }

    // if (s.endsWith("off")) {
    //   // Serial1.println("Load OFF from ID " + String(DEVICE_ID) + ": " + offImage);
    //   stby = false;
    //   stbyShown = false;
    //   Show_Image(offImage.c_str());
    //   return;
    // }

    if (s.startsWith("id") && s.length() > 2 && s.indexOf("img") == -1) {
      int pos = 2;
      while (pos < s.length() && isDigit(s[pos])) pos++;
      String cmd = s.substring(pos);

      if (cmd == "on") {
        stby = false;
        stbyShown = false;
        Show_Image(onImage.c_str());
        return;
      }

      if (cmd == "off") {
        stby = false;
        stbyShown = false;
        Show_Image(offImage.c_str());
        return;
      }
    }

    int idxIMG = s.indexOf("img");
    if (idxIMG != -1) {
      String imgName = s.substring(idxIMG + 3);
      String fullPath = "/" + imgName + ".png";

      if (!SD.exists(fullPath)) {
        Serial.println("File tidak ditemukan: " + fullPath);
        stby = true;
        stbyShown = false;
        return;
      }

      stby = false;
      stbyShown = false;
      Show_Image(fullPath.c_str());
      return;
    }
    Serial.println("Format frame tidak dikenali");
    stby = true;
    stbyShown = false;
  }

  delay(1);
}


// =======================================   CUSTOM PFP  ===========================================
bool loadConfig() {
  File f = SD.open("/config.txt");
  if (!f) {
    Serial.println("Gagal buka config.txt");
    return false;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();

    if (line.startsWith("ID=")) {
      DEVICE_ID = line.substring(3).toInt();
      Serial.print("ID Loaded: ");
      Serial.println(DEVICE_ID);
    }

    if (line.startsWith("ON=")) {
      onImage = line.substring(3);
      Serial.print("ON Image: ");
      Serial.println(onImage);
    }

    if (line.startsWith("OFF=")) {
      offImage = line.substring(4);
      Serial.print("OFF Image: ");
      Serial.println(offImage);
    }
  }

  f.close();
  return true;
}
