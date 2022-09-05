#include <ModbusMaster.h>
#include <ArduinoJson.h>

#include "Growatt124.h"
#include "Growatt305.h"
#include "GrowattTypes.h"
#include "Growatt.h"
#include "Config.h"


ModbusMaster Modbus;

// Constructor
Growatt::Growatt() {
  _eDevice = Undef_stick;
  _PacketCnt = 0;
}

void Growatt::InitProtocol(uint16_t version) {
  /**
   * @brief Initialize the protocol struct
   * @param version The version of the modbus protocol to use
   */
  if (version == 124) {
    init_growatt124(_Protocol);
  } else if (version == 305) {
    init_growatt305(_Protocol);
  } else {
    // fall back into 124 protocol when unknown version specified
    init_growatt124(_Protocol);
  }
}

void Growatt::begin(Stream &serial) {
  /**
   * @brief Set up communication with the inverter
   * @param serial The serial interface
   */
  uint8_t res;

  #if SIMULATE_INVERTER == 1
    _eDevice = SIMULATE_DEVICE;
  #else
    // init communication with the inverter
    Serial.begin(9600);
    Modbus.begin(1, serial);
    res = Modbus.readInputRegisters(0, 1);
    if(res == Modbus.ku8MBSuccess) {
      _eDevice = ShineWiFi_S; // Serial
    } else {
      delay(1000);
      Serial.begin(115200);
      Modbus.begin(1, serial);
      res = Modbus.readInputRegisters(0, 1);
      if(res == Modbus.ku8MBSuccess) {
        _eDevice = ShineWiFi_X; // USB
      }
      delay(1000);
    }
  #endif
}

eDevice_t Growatt::GetWiFiStickType() {
  /**
   * @brief After initialisation the type of the wifi stick is known
   * @returns eDevice_t type of the wifi stick
   */

  return _eDevice;
}

bool Growatt::ReadInputRegisters() {
  /**
   * @brief Read the input registers from the inverter
   * @returns true if data was read successfully, false otherwise
   */
  uint16_t registerAddress;
  uint8_t res;

  // read each fragment separately
  for (int i = 0; i < _Protocol.InputFragmentCount; i++) {
    res = Modbus.readInputRegisters(
      _Protocol.InputReadFragments[i].StartAddress,
      _Protocol.InputReadFragments[i].FragmentSize
    );
    if(res == Modbus.ku8MBSuccess) {
      for (int j = 0; j < _Protocol.InputRegisterCount; j++) {
        // make sure the register we try to read is in the fragment
        if (_Protocol.InputRegisters[j].address >= _Protocol.InputReadFragments[i].StartAddress) {
          // when we exceed the fragment size, skip to new fragment
          if (_Protocol.InputRegisters[j].address >= _Protocol.InputReadFragments[i].StartAddress + _Protocol.InputReadFragments[i].FragmentSize)
            break;
          // let's say the register address is 1013 and read window is 1000-1050
          // that means the response in the buffer is on position 1013 - 1000 = 13
          registerAddress = _Protocol.InputRegisters[j].address - _Protocol.InputReadFragments[i].StartAddress;
          if (_Protocol.InputRegisters[j].size == SIZE_16BIT) {
            _Protocol.InputRegisters[j].value = Modbus.getResponseBuffer(registerAddress);
          } else {
            _Protocol.InputRegisters[j].value = (Modbus.getResponseBuffer(registerAddress) << 16) + Modbus.getResponseBuffer(registerAddress + 1);
          }
        }
      }
    } else {
      return false;
    }
  }
  return true;
}

bool Growatt::ReadHoldingRegisters() {
  /**
   * @brief Read the holding registers from the inverter
   * @returns true if data was read successfully, false otherwise
   */
  uint16_t registerAddress;
  uint8_t res;

  // read each fragment separately
  for (int i = 0; i < _Protocol.HoldingFragmentCount; i++) {
    res = Modbus.readHoldingRegisters(
      _Protocol.HoldingReadFragments[i].StartAddress,
      _Protocol.HoldingReadFragments[i].FragmentSize
    );
    if(res == Modbus.ku8MBSuccess) {
      for (int j = 0; j < _Protocol.HoldingRegisterCount; j++) {
        if (_Protocol.HoldingRegisters[j].address >= _Protocol.HoldingReadFragments[i].StartAddress) {
          if (_Protocol.HoldingRegisters[j].address >= _Protocol.HoldingReadFragments[i].StartAddress + _Protocol.HoldingReadFragments[i].FragmentSize)
            break;
          registerAddress = _Protocol.HoldingRegisters[j].address - _Protocol.HoldingReadFragments[i].StartAddress;
          if (_Protocol.HoldingRegisters[j].size == SIZE_16BIT) {
            _Protocol.HoldingRegisters[j].value = Modbus.getResponseBuffer(registerAddress);
          } else {
            _Protocol.HoldingRegisters[j].value = (Modbus.getResponseBuffer(registerAddress) << 16) + Modbus.getResponseBuffer(registerAddress + 1);
          }
        }
      }
    } else {
      return false;
    }
  }
  return true;
}

bool Growatt::ReadData() {
  /**
   * @brief Reads the data from the inverter and updates the internal data structures
   * @returns true if data was read successfully, false otherwise
   */

  _PacketCnt++;
  _GotData = ReadInputRegisters() && ReadHoldingRegisters();
  return _GotData;
}

sGrowattModbusReg_t Growatt::GetInputRegister(uint16_t reg) {
  /**
   * @brief get the internal representation of the input register
   * @param reg the register to get
   * @returns the register value
   */
    if (_GotData == false) {
      ReadData();
    }
    return _Protocol.InputRegisters[reg];
}

sGrowattModbusReg_t Growatt::GetHoldingRegister(uint16_t reg) {
  /**
   * @brief get the internal representation of the holding register
   * @param reg the register to get
   * @returns the register value
   */
    if (_GotData == false) {
      ReadData();
    }
    return _Protocol.HoldingRegisters[reg];
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint16_t* result) {
  /**
   * @brief read 16b holding register
   * @param adr address of the register
   * @param result pointer to the result
   * @returns true if successful
   */
    uint8_t res = Modbus.readHoldingRegisters(adr, 1);
    if (res == Modbus.ku8MBSuccess) {
        *result = Modbus.getResponseBuffer(0);
        return true;
    }
    return false;
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint32_t* result) {
  /**
   * @brief read 32b holding register
   * @param adr address of the register
   * @param result pointer to the result
   * @returns true if successful
   */
    uint8_t res = Modbus.readHoldingRegisters(adr, 2);
    if (res == Modbus.ku8MBSuccess) {
        *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
        return true;
    }
    return false;
}

bool Growatt::WriteHoldingReg(uint16_t adr, uint16_t value) {
  /**
   * @brief write 16b holding register
   * @param adr address of the register
   * @param value value to write to the register
   * @returns true if successful
   */
    uint8_t res = Modbus.writeSingleRegister(adr, value);
    if (res == Modbus.ku8MBSuccess) {
        return true;
    }
    return false;
}


bool Growatt::ReadInputReg(uint16_t adr, uint16_t* result) {
  /**
   * @brief read 16b input register
   * @param adr address of the register
   * @param result pointer to the result
   * @returns true if successful
   */
    uint8_t res = Modbus.readInputRegisters(adr, 1);
    if (res == Modbus.ku8MBSuccess) {
        *result = Modbus.getResponseBuffer(0);
        return true;
    }
    return false;
}

bool Growatt::ReadInputReg(uint16_t adr, uint32_t* result) {
  /**
   * @brief read 32b input register
   * @param adr address of the register
   * @param result pointer to the result
   * @returns true if successful
   */
    uint8_t res = Modbus.readInputRegisters(adr, 2);
    if (res == Modbus.ku8MBSuccess) {
        *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
        return true;
    }
    return false;
}

void Growatt::CreateJson(char *Buffer, const char *MacAddress) {
  StaticJsonDocument<2048> doc;
  JsonObject input = doc.createNestedObject("input");
  JsonObject holding = doc.createNestedObject("holding");

#if SIMULATE_INVERTER != 1
  for (int i = 0; i < _Protocol.InputRegisterCount; i++) {
    input[_Protocol.InputRegisters[i].name] = _Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier;
  }
  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++) {
    holding[_Protocol.HoldingRegisters[i].name] = _Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier;
  }
#else
  #warning simulating the inverter
  input["Status"] = 1;
  input["DcPower"] = 230;
  input["DcVoltage"] = 70.5;
  input["DcInputCurrent"] = 8.5;
  input["AcFreq"] = 50.00;
  input["AcVoltage"] = 230.0;
  input["AcPower"] = 0.00;
  input["EnergyToday"] = 0.3;
  input["EnergyTotal"] = 49.1;
  input["OperatingTime"] = 123456;
  input["Temperature"] = 21.12;
  input["AccumulatedEnergy"] = 320;
  input["EnergyToday"] = 0.3;
  input["EnergyToday"] = 0.3;
#endif // SIMULATE_INVERTER
  doc["Mac"] = MacAddress;
  doc["Cnt"] = _PacketCnt;
  serializeJson(doc, Buffer, MQTT_PACKET_SIZE);
}

void Growatt::CreateUIJson(char *Buffer) {
  StaticJsonDocument<2048> doc;
  const char* unitStr[] = {"", "W", "kWh", "V", "A", "s", "%", "Hz", "C"};

#if SIMULATE_INVERTER != 1
  for (int i = 0; i < _Protocol.InputRegisterCount; i++) {
    if (_Protocol.InputRegisters[i].frontend == true || _Protocol.InputRegisters[i].plot == true) {
      JsonArray arr = doc.createNestedArray(_Protocol.InputRegisters[i].name);
      arr.add(_Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier); // value
      arr.add(unitStr[_Protocol.InputRegisters[i].unit]); //unit
      arr.add(_Protocol.InputRegisters[i].plot); //should be plotted
    }
  }
  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++) {
    if (_Protocol.HoldingRegisters[i].frontend == true || _Protocol.HoldingRegisters[i].plot == true) {
      JsonArray arr = doc.createNestedArray(_Protocol.HoldingRegisters[i].name);
      arr.add(_Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier);
      arr.add(unitStr[_Protocol.HoldingRegisters[i].unit]);
      arr.add(_Protocol.HoldingRegisters[i].plot); //should be plotted
    }
  }
#else
  #warning simulating the inverter
  JsonArray arr = doc.createNestedArray("Status");
  arr.add(1);arr.add("");arr.add(false);
  arr = doc.createNestedArray("DcPower");
  arr.add(230);arr.add("W");arr.add(true);
  arr = doc.createNestedArray("DcVoltage");
  arr.add(70.5);arr.add("V");arr.add(false);
  arr = doc.createNestedArray("DcInputCurrent");
  arr.add(8.5);arr.add("A");arr.add(false);
  arr = doc.createNestedArray("AcFreq");
  arr.add(50);arr.add("Hz");arr.add(false);
  arr = doc.createNestedArray("AcVoltage");
  arr.add(230);arr.add("V");arr.add(false);
  arr = doc.createNestedArray("AcPower");
  arr.add(0.00);arr.add("W");arr.add(false);
  arr = doc.createNestedArray("EnergyToday");
  arr.add(0.3);arr.add("kWh");arr.add(false);
  arr = doc.createNestedArray("EnergyTotal");
  arr.add(49.1);arr.add("kWh");arr.add(false);
  arr = doc.createNestedArray("OperatingTime");
  arr.add(123456);arr.add("s");arr.add(false);
  arr = doc.createNestedArray("Temperature");
  arr.add(21.12);arr.add("C");arr.add(false);
  arr = doc.createNestedArray("AccumulatedEnergy");
  arr.add(320);arr.add("kWh");arr.add(false);
  arr = doc.createNestedArray("EnergyToday");
  arr.add(0.3);arr.add("kWh");arr.add(false);
#endif // SIMULATE_INVERTER

  serializeJson(doc, Buffer, 4096);
}
