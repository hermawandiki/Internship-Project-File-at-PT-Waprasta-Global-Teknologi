#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Pin I2C ESP32-S3
#define SDA_PIN 8
#define SCL_PIN 9

// Pin buzzer pasif
#define BUZZER_PIN 38

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("\n=== RTC DS3231 + Buzzer per Detik (ESP32-S3) ===");

  if (!rtc.begin()) {
    Serial.println("ERROR: RTC DS3231 tidak terdeteksi!");
    while (1) delay(1000);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC kehilangan daya — set waktu dari waktu kompilasi...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Perintah:");
  Serial.println("SET YYYY-MM-DD HH:MM:SS  → untuk set waktu manual");
  Serial.println("Contoh: SET 2025-10-16 15:45:00\n");
}

void loop() {
  // Cek input serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    handleCommand(cmd);
  }

  DateTime now = rtc.now();

  // Tampilkan waktu dan suhu
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d | Suhu: %.2f °C\n",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second(),
                rtc.getTemperature());

  // Nyalakan buzzer tiap detik selama 200ms
  tone(BUZZER_PIN, 1000); // bunyi 1kHz
  delay(200);
  noTone(BUZZER_PIN);

  // Tunggu sisa waktu detik
  delay(800);
}

// === FUNGSI UNTUK MEMPROSES PERINTAH SERIAL ===
void handleCommand(String input) {
  if (input.startsWith("SET")) {
    input.replace("SET", "");
    input.trim();

    if (input.length() < 19) {
      Serial.println("Format salah! Gunakan: SET YYYY-MM-DD HH:MM:SS");
      return;
    }

    int year  = input.substring(0, 4).toInt();
    int month = input.substring(5, 7).toInt();
    int day   = input.substring(8, 10).toInt();
    int hour  = input.substring(11, 13).toInt();
    int min   = input.substring(14, 16).toInt();
    int sec   = input.substring(17, 19).toInt();

    rtc.adjust(DateTime(year, month, day, hour, min, sec));

    Serial.printf("RTC berhasil diatur ke: %04d-%02d-%02d %02d:%02d:%02d\n",
                  year, month, day, hour, min, sec);

    // Bunyi konfirmasi singkat
    tone(BUZZER_PIN, 1500);
    delay(300);
    noTone(BUZZER_PIN);
  } 
  else {
    Serial.println("Perintah tidak dikenal. Gunakan: SET YYYY-MM-DD HH:MM:SS");
  }
}
