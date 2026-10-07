#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <supla/sensor/electricity_meter.h>

// Licznik Taiye TAC1100/TAC2100 po Modbus RTU (RS485), jednofazowy.
// Rejestry wg "TAC2100 series Modbus Protocol v1.0":
//   FC04 (input registers, float, kolejność słów: wysokie słowo pierwsze):
//     0x0000 napięcie [V]        0x0006 prąd [A]
//     0x000C moc czynna [W]      0x0012 moc bierna [var]
//     0x0018 moc pozorna [VA]    0x001E współczynnik mocy
//     0x0030 częstotliwość [Hz]
//   FC03 (holding registers, ULONG, 0.01 kWh) - energie liczone całkowicie:
//     0x0400 energia czynna pobrana (import)
//     0x0402 energia czynna oddana (export)

class TAC1100 : public Supla::Sensor::ElectricityMeter {
 public:
  TAC1100(int8_t rxPin, int8_t txPin, int8_t dePin, uint8_t slaveId = 1,
          uint32_t baud = 9600)
      : serial(rxPin, txPin),
        rx(rxPin),
        tx(txPin),
        slave(slaveId),
        speed(baud) {
    deRePin = dePin;
  }

  void onInit() override {
    serial.begin(speed, SWSERIAL_8N1, rx, tx, false, 128);
    if (deRePin >= 0) {
      pinMode(deRePin, OUTPUT);
      digitalWrite(deRePin, LOW);
      node.preTransmission(preTx);
      node.postTransmission(postTx);
    }
    node.begin(slave, serial);
    Supla::Sensor::ElectricityMeter::onInit();
  }

  // Wołane cyklicznie przez ElectricityMeter (domyślnie co kilka sekund).
  // Jednostki w Supli: napięcie 0.01 V, prąd 0.001 A, moce 0.00001 kW,
  // PF 0.001, częstotliwość 0.01 Hz, energia 0.00001 kWh.
  void readValuesFromDevice() override {
    float f = 0;

    if (readFloat(0x0000, f)) setVoltage(0, (int)lroundf(f * 100.0f));
    if (readFloat(0x0006, f)) setCurrent(0, (int)lroundf(f * 1000.0f));
    if (readFloat(0x000C, f)) setPowerActive(0, (int)lroundf(f * 100.0f));
    if (readFloat(0x0012, f)) setPowerReactive(0, (int)lroundf(f * 100.0f));
    if (readFloat(0x0018, f)) setPowerApparent(0, (int)lroundf(f * 100.0f));
    if (readFloat(0x001E, f)) setPowerFactor(0, (int)lroundf(f * 1000.0f));
    if (readFloat(0x0030, f)) setFreq((int)lroundf(f * 100.0f));

    uint32_t e = 0;
    // 0.01 kWh -> 0.00001 kWh = x1000
    if (readULong(0x0400, e)) setFwdActEnergy(0, (uint64_t)e * 1000ULL);
    if (readULong(0x0402, e)) setRvrActEnergy(0, (uint64_t)e * 1000ULL);
  }

 private:
  static void preTx() { digitalWrite(deRePin, HIGH); }
  static void postTx() { digitalWrite(deRePin, LOW); }

  bool readFloat(uint16_t reg, float &out) {
    if (node.readInputRegisters(reg, 2) != node.ku8MBSuccess) return false;
    uint32_t raw = ((uint32_t)node.getResponseBuffer(0) << 16) |
                   node.getResponseBuffer(1);
    memcpy(&out, &raw, sizeof(out));
    return true;
  }

  bool readULong(uint16_t reg, uint32_t &out) {
    if (node.readHoldingRegisters(reg, 2) != node.ku8MBSuccess) return false;
    out = ((uint32_t)node.getResponseBuffer(0) << 16) |
          node.getResponseBuffer(1);
    return true;
  }

  SoftwareSerial serial;
  ModbusMaster node;
  int8_t rx, tx;
  uint8_t slave;
  uint32_t speed;
  static inline int8_t deRePin = -1;
};
