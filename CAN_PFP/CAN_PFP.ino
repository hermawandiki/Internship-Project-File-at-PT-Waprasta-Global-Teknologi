#include "driver/twai.h"

#define CAN_TX GPIO_NUM_17
#define CAN_RX GPIO_NUM_18

uint32_t id;
bool extended;
uint8_t dlc;
uint8_t buffData[8];

void setup() {
  Serial.begin(115200);
  initCAN();
}

void loop() {
  handleSerialInput();

  if (receiveMsg(&id, &extended, &dlc, buffData)) {
    //
  }
}






void initCAN() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Gagal install CAN driver");
    while (1)
      ;
  }

  if (twai_start() != ESP_OK) {
    Serial.println("Gagal start CAN driver");
    while (1)
      ;
  }
}

bool transmitMsg(uint32_t id, bool extended, uint8_t dlc, const uint8_t *data) {
  if (dlc > 8) dlc = 8;

  twai_message_t msg = {};
  msg.identifier = id;
  msg.data_length_code = dlc;
  msg.flags = 0;

  if (extended) msg.flags |= TWAI_MSG_FLAG_EXTD;

  for (uint8_t i = 0; i < dlc; i++) msg.data[i] = data[i];

  esp_err_t result = twai_transmit(&msg, pdMS_TO_TICKS(1000));

  if (result == ESP_OK) {
    Serial.printf("TX | ID=0x%03X DLC=%d DATA=",
                  id,
                  dlc);
    for (int i = 0; i < dlc; i++) Serial.printf("0x%02X ", data[i]);
    Serial.println();
    return true;
  } else {
    Serial.println("Transmit Message Failed");
    return false;
  }
}

bool receiveMsg(uint32_t *id, bool *extended, uint8_t *dlc, uint8_t *data) {
  twai_message_t rx_msg;
  esp_err_t result = twai_receive(&rx_msg, pdMS_TO_TICKS(10));

  if (result == ESP_OK) {
    *id = rx_msg.identifier;
    *extended = (rx_msg.flags & TWAI_MSG_FLAG_EXTD) != 0;
    *dlc = rx_msg.data_length_code;

    for (uint8_t i = 0; i < *dlc; i++) data[i] = rx_msg.data[i];

    Serial.printf("RX | ID=0x%03X DLC=%d DATA=",
                  *id,
                  *dlc);
    for (int i = 0; i < *dlc; i++) Serial.printf("0x%02X ", data[i]);
    Serial.println();
    return true;
  }
  return false;
}

void handleSerialInput() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  uint32_t id = 0;
  uint8_t data[8] = { 0 };
  int values[9] = { 0 };
  int count = sscanf(line.c_str(),
                     "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                     &values[0], &values[1], &values[2], &values[3],
                     &values[4], &values[5], &values[6], &values[7], &values[8]);

  if (count < 2) {
    Serial.println("Format salah! Gunakan format: 0xID 0xAA 0xBB ...");
    return;
  }

  id = values[0];
  uint8_t dlc = count - 1;

  for (uint8_t i = 0; i < dlc && i < 8; i++) {
    data[i] = (uint8_t)values[i + 1];
  }

  transmitMsg(id, true, dlc, data);
}