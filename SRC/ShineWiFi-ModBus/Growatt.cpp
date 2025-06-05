#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <TLog.h>

#include "GrowattTypes.h"
#include "Growatt.h"
#include "Config.h"
#ifndef _SHINE_CONFIG_H_
#error Please rename Config.h.example to Config.h
#endif

#if GROWATT_MODBUS_VERSION == 120
#include "Growatt120.h"
#elif GROWATT_MODBUS_VERSION == 124
#include "Growatt124.h"
#elif GROWATT_MODBUS_VERSION == 305
#include "Growatt305.h"
#elif GROWATT_MODBUS_VERSION == 3000
#include "GrowattTLXH.h"
#elif GROWATT_MODBUS_VERSION == 5000
#include "GrowattSPF.h"
#elif GROWATT_MODBUS_VERSION == 6000
#include "GrowattBP.h"
#else
#error "Unsupported Growatt Modbus version"
#endif

ModbusMaster Modbus;

// Constructor
Growatt::Growatt() {
  _eDevice = Undef_stick;
  _PacketCnt = 0;
  _PacketCntFailed = 0;

  handlers = std::map<String, CommandHandlerFunc>();

  // register default handlers
  RegisterCommand("echo", [this](const JsonDocument& req, JsonDocument& res,
                                 Growatt& inverter) {
    return handleEcho(req, res, *this);
  });

  RegisterCommand("list", [this](const JsonDocument& req, JsonDocument& res,
                                 Growatt& inverter) {
    return handleCommandList(req, res, *this);
  });

  RegisterCommand("modbus/get", [this](const JsonDocument& req,
                                       JsonDocument& res, Growatt& inverter) {
    return handleModbusGet(req, res, *this);
  });

  RegisterCommand("modbus/set", [this](const JsonDocument& req,
                                       JsonDocument& res, Growatt& inverter) {
    return handleModbusSet(req, res, *this);
  });
}

void Growatt::InitProtocol() {
/**
 * @brief Initialize the protocol struct
 * @param version The version of the modbus protocol to use
 */
#if GROWATT_MODBUS_VERSION == 120
  init_growatt120(_Protocol, *this);
#elif GROWATT_MODBUS_VERSION == 124
  init_growatt124(_Protocol, *this);
#elif GROWATT_MODBUS_VERSION == 305
  init_growatt305(_Protocol, *this);
#elif GROWATT_MODBUS_VERSION == 3000
  init_growattTLXH(_Protocol, *this);
#elif GROWATT_MODBUS_VERSION == 5000
  init_growattSPF(_Protocol, *this);
#elif GROWATT_MODBUS_VERSION == 6000
  init_growattBP(_Protocol, *this);
#else
#error "Unsupported Growatt Modbus version"
#endif
}

void Growatt::begin(Stream& serial) {
  /**
   * @brief Set up communication with the inverter
   * @param serial The serial interface
   */
#if SIMULATE_INVERTER == 1
  _eDevice = SIMULATE_DEVICE;
#else
  uint8_t res;
  // init communication with the inverter
  Serial.begin(9600);
  Modbus.begin(1, serial);
  res = Modbus.readInputRegisters(0, 1);
  if (res == Modbus.ku8MBSuccess) {
    _eDevice = ShineWiFi_S;  // Serial
  } else {
    delay(1000);
    Serial.begin(115200);
    Modbus.begin(1, serial);
    Modbus.setResponseTimeout(250);
    res = Modbus.readInputRegisters(0, 1);
    if (res == Modbus.ku8MBSuccess) {
      _eDevice = ShineWiFi_X;  // USB
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

bool Growatt::ReadInputRegisters(uint8_t& i) {
  /**
   * @brief Read the input registers from the inverter
   * @returns true if data was read successfully, false otherwise
   */
  uint16_t registerAddress;
  uint8_t res;
  int j = 0;

  // read each fragment separately
  for (; i < _Protocol.InputFragmentCount; i++) {
#ifdef DEBUG_MODBUS_OUTPUT
    Log.printf("Modbus: read Segment from 0x%02X with len: %d ...",
               _Protocol.InputReadFragments[i].StartAddress,
               _Protocol.InputReadFragments[i].FragmentSize);
#endif
    res =
        Modbus.readInputRegisters(_Protocol.InputReadFragments[i].StartAddress,
                                  _Protocol.InputReadFragments[i].FragmentSize);
    if (res == Modbus.ku8MBSuccess) {
#ifdef DEBUG_MODBUS_OUTPUT
      Log.println(F("ok"));
#endif
      for (; j < _Protocol.InputRegisterCount; j++) {
        // make sure the register we try to read is in the fragment
        if (_Protocol.InputRegisters[j].address >=
            _Protocol.InputReadFragments[i].StartAddress) {
          // when we exceed the fragment size, skip to new fragment
          if (_Protocol.InputRegisters[j].address >=
              _Protocol.InputReadFragments[i].StartAddress +
                  _Protocol.InputReadFragments[i].FragmentSize)
            break;
          // let's say the register address is 1013 and read window is 1000-1050
          // that means the response in the buffer is on position 1013 - 1000 =
          // 13
          registerAddress = _Protocol.InputRegisters[j].address -
                            _Protocol.InputReadFragments[i].StartAddress;
          if (_Protocol.InputRegisters[j].size == SIZE_16BIT ||
              _Protocol.InputRegisters[j].size == SIZE_16BIT_S) {
            _Protocol.InputRegisters[j].value =
                Modbus.getResponseBuffer(registerAddress);
          } else {
            _Protocol.InputRegisters[j].value =
                (Modbus.getResponseBuffer(registerAddress) << 16) +
                Modbus.getResponseBuffer(registerAddress + 1);
          }
        }
      }
    } else {
#ifdef DEBUG_MODBUS_OUTPUT
      Log.println(F("failed"));
#endif
      return false;
    }
  }
#if GROWATT_MODBUS_VERSION == 3000
  _Protocol.InputRegisters[P3000_INVERTER_STATUS].value &= 0xff;
  _Protocol.InputRegisters[P3000_INVERTER_RUNSTATE].value >>= 8;
  _Protocol.InputRegisters[P3000_BDC_SYSSTATE].value &= 0xff;
  _Protocol.InputRegisters[P3000_BDC_SYSMODE].value >>= 8;
#endif
  return true;
}

bool Growatt::ReadHoldingRegisters(uint8_t& i) {
  /**
   * @brief Read the holding registers from the inverter
   * @returns true if data was read successfully, false otherwise
   */
  uint16_t registerAddress;
  uint8_t res;

  // read each fragment separately
  for (; i < _Protocol.HoldingFragmentCount; i++) {
    res = Modbus.readHoldingRegisters(
        _Protocol.HoldingReadFragments[i].StartAddress,
        _Protocol.HoldingReadFragments[i].FragmentSize);
    if (res == Modbus.ku8MBSuccess) {
      for (int j = 0; j < _Protocol.HoldingRegisterCount; j++) {
        if (_Protocol.HoldingRegisters[j].address >=
            _Protocol.HoldingReadFragments[i].StartAddress) {
          if (_Protocol.HoldingRegisters[j].address >=
              _Protocol.HoldingReadFragments[i].StartAddress +
                  _Protocol.HoldingReadFragments[i].FragmentSize)
            break;
          registerAddress = _Protocol.HoldingRegisters[j].address -
                            _Protocol.HoldingReadFragments[i].StartAddress;
          if (_Protocol.HoldingRegisters[j].size == SIZE_16BIT ||
              _Protocol.HoldingRegisters[j].size == SIZE_16BIT_S) {
            _Protocol.HoldingRegisters[j].value =
                Modbus.getResponseBuffer(registerAddress);
          } else {
            _Protocol.HoldingRegisters[j].value =
                (Modbus.getResponseBuffer(registerAddress) << 16) +
                Modbus.getResponseBuffer(registerAddress + 1);
          }
        }
      }
    } else {
      return false;
    }
  }
  return true;
}

bool Growatt::ReadData(uint8_t maxRetries) {
  /**
   * @brief Reads the data from the inverter and updates the internal data
   * structures
   * @returns true if data was read successfully, false otherwise
   */
  uint8_t inputFragOffs = 0;
  uint8_t holdingFragOffs = 0;
  uint8_t retryCnt = 0;
  unsigned long readStart = millis();
  bool res;
  while (inputFragOffs < _Protocol.InputFragmentCount &&
         retryCnt < maxRetries) {
    _PacketCnt++;
    res = ReadInputRegisters(inputFragOffs);
    if (!res) {
      _PacketCntFailed++;
      retryCnt++;
    }
  }
  while (holdingFragOffs < _Protocol.HoldingFragmentCount &&
         retryCnt < maxRetries) {
    _PacketCnt++;
    res = ReadHoldingRegisters(holdingFragOffs);
    if (!res) {
      _PacketCntFailed++;
      retryCnt++;
    }
  }
  _GotData = retryCnt < maxRetries;
  if (_GotData) {
    Log.print(F("ReadData() successful after "));
  } else {
    Log.print(F("ReadData() NOT successful after "));
  }
  Log.print(retryCnt);
  Log.print(F(" retries in "));
  Log.print(millis() - readStart);
  Log.println(F("ms"));
  return _GotData;
}

sGrowattModbusReg_t Growatt::GetInputRegister(uint16_t reg) {
  /**
   * @brief get the internal representation of the input register
   * @param reg the register to get
   * @returns the register value
   */
  if (_GotData == false) {
    ReadData(1);
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
    ReadData(1);
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
#if SIMULATE_INVERTER != 1
  uint8_t res = Modbus.readHoldingRegisters(adr, 1);
  if (res == Modbus.ku8MBSuccess) {
    *result = Modbus.getResponseBuffer(0);
    return true;
  }
  return false;
#else
  *result = 0;
  return true;
#endif
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint32_t* result) {
/**
 * @brief read 32b holding register
 * @param adr address of the register
 * @param result pointer to the result
 * @returns true if successful
 */
#if SIMULATE_INVERTER != 1
  uint8_t res = Modbus.readHoldingRegisters(adr, 2);
  if (res == Modbus.ku8MBSuccess) {
    *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
    return true;
  }
  return false;
#else
  *result = 0;
  return true;
#endif
}

bool Growatt::ReadHoldingRegFrag(uint16_t adr, uint8_t size, uint16_t* result) {
  /**
   * @brief read 16b holding register fragment
   * @param adr address of the register
   * @param size size of the register
   * @param result pointer to the result
   * @returns true if successful
   */
  uint8_t res = Modbus.readHoldingRegisters(adr, size);
  if (res == Modbus.ku8MBSuccess) {
    for (int i = 0; i < size; i++) {
      result[i] = Modbus.getResponseBuffer(i);
    }
    return true;
  }
  return false;
}

bool Growatt::ReadHoldingRegFrag(uint16_t adr, uint8_t size, uint32_t* result) {
  /**
   * @brief read 32b holding register fragment
   * @param adr address of the register
   * @param size size of the register
   * @param result pointer to the result
   * @returns true if successful
   */
  uint8_t res = Modbus.readHoldingRegisters(adr, size * 2);
  if (res == Modbus.ku8MBSuccess) {
    for (int i = 0; i < size; i++) {
      result[i] = (Modbus.getResponseBuffer(i * 2) << 16) +
                  Modbus.getResponseBuffer(i * 2 + 1);
    }
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
#if SIMULATE_INVERTER != 1
  uint8_t res = Modbus.writeSingleRegister(adr, value);
  if (res == Modbus.ku8MBSuccess) {
    return true;
  }
  return false;
#else
  return true;
#endif
}

bool Growatt::WriteHoldingRegFrag(uint16_t adr, uint8_t size, uint16_t* value) {
  /**
   * @brief write 16b holding register
   * @param adr address of the register
   * @param value value to write to the register
   * @param size size of the register
   * @returns true if successful
   */
  for (int i = 0; i < size; i++) {
    Modbus.setTransmitBuffer(i, value[i]);
  }
  uint8_t res = Modbus.writeMultipleRegisters(adr, size);
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
#if SIMULATE_INVERTER != 1
  uint8_t res = Modbus.readInputRegisters(adr, 1);
  if (res == Modbus.ku8MBSuccess) {
    *result = Modbus.getResponseBuffer(0);
    return true;
  }
  return false;
#else
  *result = 0;
  return true;
#endif
}

bool Growatt::ReadInputReg(uint16_t adr, uint32_t* result) {
/**
 * @brief read 32b input register
 * @param adr address of the register
 * @param result pointer to the result
 * @returns true if successful
 */
#if SIMULATE_INVERTER != 1
  uint8_t res = Modbus.readInputRegisters(adr, 2);
  if (res == Modbus.ku8MBSuccess) {
    *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
    return true;
  }
  return false;
#else
  *result = 0;
  return true;
#endif
}

double Growatt::roundByResolution(const double& value,
                                  const float& resolution) {
  double res = 1 / resolution;
  return int32_t(value * res + 0.5) / res;
}

double Growatt::getRegValue(sGrowattModbusReg_t* reg) {
  double result = 0;
  RegisterSize_t size = reg->size;
  const float& mult = reg->multiplier;
  const uint32_t& value = reg->value;
  const float& resolution = reg->resolution;

  switch (size) {
    case SIZE_16BIT_S:
      result = (mult == (int)mult)
                   ? (int16_t)value * mult
                   : roundByResolution((int16_t)value * mult, resolution);
      break;
    case SIZE_32BIT_S:
      result = (mult == (int)mult)
                   ? (int32_t)value * mult
                   : roundByResolution((int32_t)value * mult, resolution);
      break;
    default:
      result = (mult == (int)mult)
                   ? value * mult
                   : roundByResolution(value * mult, resolution);
  }
  return result;
}

bool Growatt::GetSingleValueByName(const String& name, double& value) {
  for (int i = 0; i < _Protocol.InputRegisterCount; i++) {
    if (name.equalsIgnoreCase(_Protocol.InputRegisters[i].name)) {
      value = getRegValue(&_Protocol.InputRegisters[i]);
      return true;
    }
  }
  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++) {
    if (name.equalsIgnoreCase(_Protocol.HoldingRegisters[i].name)) {
      value = getRegValue(&_Protocol.HoldingRegisters[i]);
      return true;
    }
  }
  return false;
}

void Growatt::CreateJson(JsonDocument& doc, const String& MacAddress,
                         const String& Hostname) {
  if (!Hostname.isEmpty()) {
    doc["Hostname"] = Hostname;
  }
#if SIMULATE_INVERTER != 1
  for (int i = 0; i < _Protocol.InputRegisterCount; i++)
    doc[_Protocol.InputRegisters[i].name] =
        getRegValue(&_Protocol.InputRegisters[i]);

  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++)
    doc[_Protocol.HoldingRegisters[i].name] =
        getRegValue(&_Protocol.HoldingRegisters[i]);
#else
#warning simulating the inverter
  doc["Status"] = 1;
  doc["DcPower"] = 230;
  doc["DcVoltage"] = 70.5;
  doc["DcInputCurrent"] = 8.5;
  doc["AcFreq"] = 50.00;
  doc["AcVoltage"] = 230.0;
  doc["AcPower"] = 0.00;
  doc["EnergyToday"] = 0.3;
  doc["EnergyTotal"] = 49.1;
  doc["OperatingTime"] = 123456;
  doc["Temperature"] = 21.12;
  doc["AccumulatedEnergy"] = 320;
#endif  // SIMULATE_INVERTER
  doc["Mac"] = MacAddress;
  doc["Cnt"] = _PacketCnt;
  doc["CntFailed"] = _PacketCntFailed;
  doc["Uptime"] = millis() / 1000;
  doc["WifiRSSI"] = WiFi.RSSI();
  doc["HeapFree"] = ESP.getFreeHeap();
#ifdef ESP32
  doc["HeapSize"] = ESP.getHeapSize();
  doc["HeapMaxAlloc"] = ESP.getMaxAllocHeap();
  doc["HeapMinFree"] = ESP.getMinFreeHeap();
  doc["HeapFragmentation"] =
      100 - (100 * ESP.getMaxAllocHeap() / ESP.getFreeHeap());
#else
  static uint32_t heap_min_free = ESP.getFreeHeap();
  heap_min_free = min(ESP.getFreeHeap(), heap_min_free);
  doc["HeapMaxAlloc"] = ESP.getMaxFreeBlockSize();
  doc["HeapMinFree"] = heap_min_free;
  doc["HeapFragmentation"] = ESP.getHeapFragmentation();
#endif

  if (doc.overflowed()) {
    Log.println(
        F("WARN CreateJson: JsonDocument overflowed! Output will be "
          "truncated."));
  }
}

void Growatt::CreateUIJson(JsonDocument& doc, const String& Hostname) {
#if SIMULATE_INVERTER != 1
  const char* unitStr[] = {"",   "W",  "kWh", "V",  "A",    "s",  "%",
                           "Hz", "°C", "VA",  "mA", "kOhm", "var"};
  const char* statusStr[] = {"(Waiting)", "(Normal Operation)", "", "(Error)"};
  const int statusStrLength = sizeof(statusStr) / sizeof(char*);
  const char* priorityStr[] = {"(Load First)", "(Battery First)",
                               "(Grid First)"};
  const int priorityStrLength = sizeof(priorityStr) / sizeof(char*);
  const char* bdcModeStr[] = {"(idle)", "(charging)", "(discharging)"};
  const int bdcModeStrLength = sizeof(bdcModeStr) / sizeof(char*);

  if (!Hostname.isEmpty()) {
    JsonArray arr = doc.createNestedArray("Hostname");
    arr.add(Hostname);
    arr.add("");
    arr.add(false);
  }

  for (int i = 0; i < _Protocol.InputRegisterCount; i++) {
    if (_Protocol.InputRegisters[i].frontend == true ||
        _Protocol.InputRegisters[i].plot == true) {
      JsonArray arr = doc.createNestedArray(_Protocol.InputRegisters[i].name);

      // value
      arr.add(getRegValue(&_Protocol.InputRegisters[i]));

      if ((String(_Protocol.InputRegisters[i].name) == F("InverterStatus") ||
           String(_Protocol.InputRegisters[i].name) == F("BDCSysState")) &&
          _Protocol.InputRegisters[i].value < statusStrLength) {
        arr.add(statusStr[_Protocol.InputRegisters[i].value]);  // use unit for
                                                                // status
      } else if (String(_Protocol.InputRegisters[i].name) == F("BDCSysMode") &&
                 _Protocol.InputRegisters[i].value < bdcModeStrLength) {
        arr.add(bdcModeStr[_Protocol.InputRegisters[i].value]);
      } else if (String(_Protocol.InputRegisters[i].name) == F("Priority") &&
                 _Protocol.InputRegisters[i].value < priorityStrLength) {
        arr.add(priorityStr[_Protocol.InputRegisters[i].value]);
      } else {
        arr.add(unitStr[_Protocol.InputRegisters[i].unit]);  // unit
      }
      arr.add(_Protocol.InputRegisters[i].plot);  // should be plotted
    }
  }
  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++) {
    if (_Protocol.HoldingRegisters[i].frontend == true ||
        _Protocol.HoldingRegisters[i].plot == true) {
      JsonArray arr = doc.createNestedArray(_Protocol.HoldingRegisters[i].name);

      // value
      arr.add(getRegValue(&_Protocol.HoldingRegisters[i]));

      if (String(_Protocol.HoldingRegisters[i].name) == F("InverterStatus") &&
          _Protocol.HoldingRegisters[i].value < statusStrLength) {
        arr.add(statusStr[_Protocol.HoldingRegisters[i].value]);  // use unit
                                                                  // for status
      } else {
        arr.add(unitStr[_Protocol.HoldingRegisters[i].unit]);  // unit
      }
      arr.add(_Protocol.HoldingRegisters[i].plot);  // should be plotted
    }
  }
#else
#warning simulating the inverter
  JsonArray arr = doc.createNestedArray("Status");
  arr.add(1);
  arr.add("(Normal Operation)");
  arr.add(false);
  arr = doc.createNestedArray("DcPower");
  arr.add(230);
  arr.add("W");
  arr.add(true);
  arr = doc.createNestedArray("DcVoltage");
  arr.add(70.5);
  arr.add("V");
  arr.add(false);
  arr = doc.createNestedArray("DcInputCurrent");
  arr.add(8.5);
  arr.add("A");
  arr.add(false);
  arr = doc.createNestedArray("AcFreq");
  arr.add(50);
  arr.add("Hz");
  arr.add(false);
  arr = doc.createNestedArray("AcVoltage");
  arr.add(230);
  arr.add("V");
  arr.add(false);
  arr = doc.createNestedArray("AcPower");
  arr.add(0.00);
  arr.add("W");
  arr.add(false);
  arr = doc.createNestedArray("EnergyToday");
  arr.add(0.3);
  arr.add("kWh");
  arr.add(false);
  arr = doc.createNestedArray("EnergyTotal");
  arr.add(49.1);
  arr.add("kWh");
  arr.add(false);
  arr = doc.createNestedArray("OperatingTime");
  arr.add(123456);
  arr.add("s");
  arr.add(false);
  arr = doc.createNestedArray("Temperature");
  arr.add(21.12);
  arr.add("C");
  arr.add(false);
  arr = doc.createNestedArray("AccumulatedEnergy");
  arr.add(320);
  arr.add("kWh");
  arr.add(false);
#endif  // SIMULATE_INVERTER

  if (doc.overflowed()) {
    Log.println(
        F("WARN CreateUIJson: JsonDocument overflowed! Output will be "
          "truncated."));
  }
}

void Growatt::camelCaseToSnakeCase(const String& input, char* output) {
  int outputIndex = 0;
  for (uint i = 0; input[i] != '\0'; i++) {
    if (i > 0 && isUpperCase(input[i]) &&
        (isLowerCase(input[i - 1]) ||
         (i < input.length() - 1 && isLowerCase(input[i + 1])))) {
      output[outputIndex++] = '_';
    }
    output[outputIndex++] = toLowerCase(input[i]);
  }
  output[outputIndex] = '\0';
}

void Growatt::metricsAddValue(const String& name, const double& value,
                              const float& resolution, String& metrics,
                              const String& labels) {
  const String svalue =
      (resolution == 1) ? String((int32_t)value) : String(value);
  char nameSnakeCase[name.length() + 10];
  camelCaseToSnakeCase(name, nameSnakeCase);
  metrics +=
      "growatt_" + String(nameSnakeCase) + "{" + labels + "} " + svalue + "\n";
}

void Growatt::CreateMetrics(String& metrics, const String& MacAddress,
                            const String& Hostname) {
  String labels;
  if (Hostname == DEFAULT_HOSTNAME) {
    labels = "mac=\"" + MacAddress + "\"";
  } else {
    labels = "mac=\"" + MacAddress + "\",name=\"" + Hostname + "\"";
  }
#if SIMULATE_INVERTER != 1
  for (int i = 0; i < _Protocol.InputRegisterCount; i++)
    metricsAddValue(_Protocol.InputRegisters[i].name,
                    getRegValue(&_Protocol.InputRegisters[i]),
                    _Protocol.InputRegisters[i].resolution, metrics, labels);

  for (int i = 0; i < _Protocol.HoldingRegisterCount; i++)
    metricsAddValue(_Protocol.HoldingRegisters[i].name,
                    getRegValue(&_Protocol.HoldingRegisters[i]),
                    _Protocol.HoldingRegisters[i].resolution, metrics, labels);

#else
#warning simulating the inverter
  metricsAddValue("Status", 1, 1, metrics, labels);
  metricsAddValue("DcPower", 230, 0.1, metrics, labels);
  metricsAddValue("DcVoltage", 70.5, 0.1, metrics, labels);
  metricsAddValue("DcInputCurrent", 8.5, 0.1, metrics, labels);
  metricsAddValue("AcFreq", 50.00, 0.01, metrics, labels);
  metricsAddValue("AcVoltage", 230.0, 0.1, metrics, labels);
  metricsAddValue("AcPower", 0.0, 0.1, metrics, labels);
  metricsAddValue("EnergyToday", 0.3, 0.1, metrics, labels);
  metricsAddValue("EnergyTotal", 49.1, 0.1, metrics, labels);
  metricsAddValue("OperatingTime", 123456, 0.1, metrics, labels);
  metricsAddValue("Temperature", 21.12, 0.1, metrics, labels);
  metricsAddValue("AccumulatedEnergy", 320, 0.1, metrics, labels);
#endif  // SIMULATE_INVERTER
  metricsAddValue("Cnt", _PacketCnt, 1, metrics, labels);
  metricsAddValue("CntFailed", _PacketCntFailed, 1, metrics, labels);
  metricsAddValue("Uptime", millis() / 1000, 1, metrics, labels);
  metricsAddValue("WifiRSSI", WiFi.RSSI(), 1, metrics, labels);

  metricsAddValue("HeapFree", ESP.getFreeHeap(), 1, metrics, labels);
#ifdef ESP32
  metricsAddValue("HeapSize", ESP.getHeapSize(), 1, metrics, labels);
  metricsAddValue("HeapMaxAlloc", ESP.getMaxAllocHeap(), 1, metrics, labels);
  metricsAddValue("HeapMinFree", ESP.getMinFreeHeap(), 1, metrics, labels);
  metricsAddValue("HeapFragmentation",
                  100 - (100 * ESP.getMaxAllocHeap() / ESP.getFreeHeap()), 1,
                  metrics, labels);
#else
  static uint32_t heap_min_free = ESP.getFreeHeap();
  heap_min_free = min(ESP.getFreeHeap(), heap_min_free);
  metricsAddValue("HeapMaxAlloc", ESP.getMaxFreeBlockSize(), 1, metrics,
                  labels);
  metricsAddValue("HeapMinFree", heap_min_free, 1, metrics, labels);
  metricsAddValue("HeapFragmentation", ESP.getHeapFragmentation(), 1, metrics,
                  labels);
#endif
}

void Growatt::RegisterCommand(const String& command,
                              CommandHandlerFunc handler) {
  handlers[command] = handler;
}

void Growatt::HandleCommand(const String& command, const byte* payload,
                            const unsigned int length, JsonDocument& req,
                            JsonDocument& res) {
  String correlationId = "";
  DeserializationError deserializationErr =
      deserializeJson(req, payload, length);

  bool success;
  String message;
  if (deserializationErr) {
    Log.println("Failed to parse JSON request in command '" + command +
                "': " + String(deserializationErr.c_str()));
    success = false;
    message =
        "Failed to parse JSON request: " + String(deserializationErr.c_str());
  } else {
    if (req.containsKey("correlationId")) {
      res["correlationId"] = String(req["correlationId"].as<String>());
    }

    auto it = handlers.find(command.c_str());
    if (it != handlers.end()) {
      Log.println("Handling command: " + command);
      std::tie(success, message) = it->second(req, res, *this);
    } else {
      Log.println("Unknown command: " + command);
      success = false;
      message = "Unknown command: " + command;
    }
  }

  res["command"] = command;
  res["success"] = success;
  res["message"] = message;
}

std::tuple<bool, String> Growatt::handleEcho(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter) {
  if (!req.containsKey("text")) {
    return std::make_tuple(false, "'text' field is required");
  }
  String text = req["text"].as<String>();
  res["text"] = "Echo: " + text;
  return std::make_tuple(true, "");
}

std::tuple<bool, String> Growatt::handleCommandList(const JsonDocument& req,
                                                    JsonDocument& res,
                                                    Growatt& inverter) {
  JsonArray commands = res.createNestedArray("commands");
  for (auto it = handlers.begin(); it != handlers.end(); ++it) {
    commands.add(it->first);
  }
  return std::make_tuple(true, "");
}

std::tuple<bool, String> Growatt::handleModbusGet(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter) {
  if (!req.containsKey("id")) {
    return std::make_tuple(false, "'id' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t id = req["id"].as<uint16_t>();
#endif

  if (!req.containsKey("type")) {
    return std::make_tuple(false, "'type' field is required");
  }

  String type = req["type"].as<String>();

  if (type != "16b" && type != "32b") {
    return std::make_tuple(false, "'type' must be '16b' or '32b'");
  }

  if (!req.containsKey("registerType")) {
    return std::make_tuple(false, "'registerType' field is required");
  }

  String registerType = req["registerType"].as<String>();

  if (registerType != "H" && registerType != "I") {
    return std::make_tuple(
        false, "'registerType' must be 'H' (holding) or 'I' (input)");
  }

#if SIMULATE_INVERTER != 1
  if (type == "16b") {
    uint16_t value;
    if (registerType == "H") {
      if (!inverter.ReadHoldingReg(id, &value)) {
        return std::make_tuple(false, "Failed to read holding register");
      }
    } else {
      if (!inverter.ReadInputReg(id, &value)) {
        return std::make_tuple(false, "Failed to read input register");
      }
    }
    res["value"] = value;
  } else {
    uint32_t value;
    if (registerType == "H") {
      if (!inverter.ReadHoldingReg(id, &value)) {
        return std::make_tuple(false, "Failed to read holding register");
      }
    } else {
      if (!inverter.ReadInputReg(id, &value)) {
        return std::make_tuple(false, "Failed to read input register");
      }
    }
    res["value"] = value;
  }
#else
  if (type == "16b") {
    res["value"] = 16;
  } else {
    res["value"] = 32;
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> Growatt::handleModbusSet(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter) {
  if (!req.containsKey("id")) {
    return std::make_tuple(false, "'id' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t id = req["id"].as<uint16_t>();
#endif

  if (!req.containsKey("type")) {
    return std::make_tuple(false, "'type' field is required");
  }

  String type = req["type"].as<String>();

  if (type == "32b") {
    return std::make_tuple(
        false, "writing to double (32b) registers is not currently supported");
  }

  if (type != "16b") {
    return std::make_tuple(false, "'type' must be '16b'");
  }

  if (!req.containsKey("registerType")) {
    return std::make_tuple(false, "'registerType' field is required");
  }

  String registerType = req["registerType"].as<String>();

  if (registerType == "I") {
    return std::make_tuple(false,
                           "it is not possible to write into input registers");
  }

  if (registerType != "H") {
    return std::make_tuple(false, "'registerType' must be 'H' (holding)");
  }

  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
#endif

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(id, value)) {
    return std::make_tuple(false, "failed to write holding register");
  }
#endif

  return std::make_tuple(true, "success");
}
