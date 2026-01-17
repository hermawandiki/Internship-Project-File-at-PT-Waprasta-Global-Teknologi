#include "driver/twai.h"

// Daftar bitrate yang akan dicoba
twai_timing_config_t timingConfigs[] = {
  TWAI_TIMING_CONFIG_25KBITS(),
  TWAI_TIMING_CONFIG_50KBITS(),
  TWAI_TIMING_CONFIG_100KBITS(),
  TWAI_TIMING_CONFIG_125KBITS(),
  TWAI_TIMING_CONFIG_250KBITS(),
  TWAI_TIMING_CONFIG_500KBITS(),
  TWAI_TIMING_CONFIG_800KBITS(),
  TWAI_TIMING_CONFIG_1MBITS()
};

const char* timingNames[] = {
  "25 kbps", "50 kbps", "100 kbps", "125 kbps",
  "250 kbps", "500 kbps", "800 kbps", "1 Mbps"
};

#define NUM_TIMINGS (sizeof(timingConfigs) / sizeof(twai_timing_config_t))

// Pin ESP32 ke VP230
#define CAN_TX GPIO_NUM_17
#define CAN_RX GPIO_NUM_18

void setup() {
  Serial.begin(115200);
  Serial.println("CAN Bitrate Scanner");
}

void loop() {
  for (int i = 0; i < NUM_TIMINGS; i++) {
    Serial.print("Coba bitrate: ");
    Serial.println(timingNames[i]);

    // Konfigurasi CAN
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install driver
    if (twai_driver_install(&g_config, &TWAI_TIMING_CONFIG_500KBITS(), &f_config) != ESP_OK) {
      Serial.println("  Gagal install driver");
      continue;
    }
    if (twai_start() != ESP_OK) {
      Serial.println("  Gagal start CAN");
      twai_driver_uninstall();
      continue;
    }

    // Tunggu data 2 detik
    twai_message_t message;
    bool gotData = false;
    unsigned long start = millis();
    while (millis() - start < 2000) {
      if (twai_receive(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
        Serial.print("  ✅ Data terdeteksi pada ");
        Serial.println(timingNames[i]);

        Serial.print("     ID: 0x");
        Serial.print(message.identifier, HEX);
        Serial.print(" DLC: ");
        Serial.print(message.data_length_code);
        Serial.print(" Data: ");
        for (int j = 0; j < message.data_length_code; j++) {
          Serial.printf("%02X ", message.data[j]);
        }
        Serial.println();

        gotData = true;
        break;
      }
    }

    // Hentikan driver
    twai_stop();
    twai_driver_uninstall();

    if (gotData) {
      Serial.println("\n=== Bitrate terdeteksi, hentikan scan ===");
      while (1); // berhenti scan
    }
  }

  Serial.println("⚠️ Tidak ada data terdeteksi di semua bitrate, coba ulangi...");
  delay(5000); // coba lagi setelah 5 detik
}
