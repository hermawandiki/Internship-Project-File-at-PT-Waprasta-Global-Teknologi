#include <SoftwareSerial.h>

// UART pin
#define RX_PIN 10
#define TX_PIN 9

SoftwareSerial ModbusSerial(RX_PIN, TX_PIN);

// Frame Modbus
uint8_t readDIN[] = { 0x05, 0x02, 0x00, 0x00, 0x00, 0x01, 0xB8, 0x4E };
uint8_t writeCoilOn[]  = { 0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A };
uint8_t writeCoilOff[] = { 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA };

uint8_t dinState = 0;

void setup() {
  Serial.begin(9600);
  ModbusSerial.begin(9600);
}

void loop() {
  // kirim permintaan baca input
  ModbusSerial.write(readDIN, sizeof(readDIN));
  delay(20);  // beri waktu slave balas

  // baca byte masuk satu per satu
  uint8_t buffer[32];
  int idx = 0;

  while (ModbusSerial.available() && idx < sizeof(buffer)) {
    buffer[idx++] = ModbusSerial.read();
  }

  // cek minimal panjang frame
  if (idx >= 5 && buffer[1] == 0x02) {
    dinState = buffer[3] & 0x01;

    if (dinState) {
      ModbusSerial.write(writeCoilOn, sizeof(writeCoilOn));
      Serial.println("ON");
    } else {
      ModbusSerial.write(writeCoilOff, sizeof(writeCoilOff));
      Serial.println("OFF");
    }
  }

  // tampilkan data balasan dari slave
  for (uint8_t i = 0; i < idx; i++) {
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  delay(100);  // jeda antar request (atur sesuai kebutuhan)
}
