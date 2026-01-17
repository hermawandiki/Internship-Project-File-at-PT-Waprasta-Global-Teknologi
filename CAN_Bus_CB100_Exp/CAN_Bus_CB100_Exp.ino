#include "driver/twai.h"

#define CAN_TX GPIO_NUM_17
#define CAN_RX GPIO_NUM_18

void setup() {
  Serial.begin(115200);
  Serial.println("CAN Receiver @ 500 kbps");

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Gagal install CAN driver");
    while (1);
  }

  if (twai_start() != ESP_OK) {
    Serial.println("Gagal start CAN driver");
    while (1);
  }

  Serial.println("CAN driver siap, menunggu data...");
}

void loop() {
  twai_message_t message;

  if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.printf("%03X ", message.identifier); // ID
    for (int i = 0; i < message.data_length_code; i++) {
      Serial.printf("%02X ", message.data[i]); // Data
    }

    // KETERANGAN DATA
    if (message.identifier == 0x100 && message.data_length_code == 2) {
      if (message.data[0] == 0x11) {
        Serial.printf("\t\t\tTombol %d DITEKAN", message.data[1]);
      } else if (message.data[0] == 0x12) {
        Serial.printf("\t\t\tTombol %d DILEPAS", message.data[1]);
      }
    }
    else if (message.identifier == 0x001 && message.data_length_code == 8) {
      uint8_t ledMask = message.data[1];
      if(message.data[0] == 0x01) Serial.print("\tLED ON  : ");
      if(message.data[0] == 0x02) Serial.print("\tLED OFF : ");
      for (int bit = 0; bit < 8; bit++) {
        if (ledMask & (1 << bit)) {
          Serial.printf("%d ", bit + 1);
        }
      }
    }
    else if (message.identifier == 0xFF) {
      Serial.printf("\t\t\t\tTASK COMPLETED");
    }

    Serial.println();
  }
}
