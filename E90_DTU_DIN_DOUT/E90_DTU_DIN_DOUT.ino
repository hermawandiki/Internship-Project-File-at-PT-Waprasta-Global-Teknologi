#include <HardwareSerial.h>

HardwareSerial ModbusSerial(1);  // gunakan UART1

// UART pin
#define TX_PIN 17
#define RX_PIN 16

// Frame Modbus
uint8_t readDIN[] = { 0x05, 0x02, 0x00, 0x00, 0x00, 0x01, 0xB8, 0x4E };
uint8_t writeCoilOn[] = { 0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A };
uint8_t writeCoilOff[] = { 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA };
uint8_t dinState, lastDinState = 0;

void setup() {
  Serial.begin(9600);
  ModbusSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  ModbusSerial.write(readDIN, sizeof(readDIN));

  // Cek apakah ada respon masuk
  if (ModbusSerial.available()) {
    uint8_t buffer[32];
    int len = ModbusSerial.readBytes(buffer, sizeof(buffer));

    // Minimal panjang frame valid (slave+func+bytecount+data+CRC) = 5
    if (len >= 5 && buffer[1] == 0x02) {
      dinState = buffer[3] & 0x01;
      if (dinState != lastDinState) {
        if (dinState) {
          ModbusSerial.write(writeCoilOn, sizeof(writeCoilOn));
          Serial.println("ON");
        } else {
          ModbusSerial.write(writeCoilOff, sizeof(writeCoilOff));
          Serial.println("OFF");
        }
      } else {
        // nothing
      }
    }
  }
  lastDinState = dinState;
  delay(50);
}
