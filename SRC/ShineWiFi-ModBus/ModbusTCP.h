// new
#ifndef _MODBUS_TCP_H_
#define _MODBUS_TCP_H_

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

class ModbusTCP {
private:
  WiFiServer* server;
  WiFiClient client;
  uint16_t port;
  bool enabled;

  // Modbus function codes
  static const uint8_t FC_READ_HOLDING_REGISTERS = 0x03;
  static const uint8_t FC_READ_INPUT_REGISTERS = 0x04;
  static const uint8_t FC_WRITE_SINGLE_REGISTER = 0x06;

  // Exception codes
  static const uint8_t EX_ILLEGAL_FUNCTION = 0x01;
  static const uint8_t EX_ILLEGAL_DATA_ADDRESS = 0x02;
  static const uint8_t EX_ILLEGAL_DATA_VALUE = 0x03;

  uint8_t requestBuffer[260];
  uint8_t responseBuffer[260];

  uint16_t calculateCRC(uint8_t* buffer, uint16_t length);
  void sendException(uint8_t functionCode, uint8_t exceptionCode);
  void processRequest();

public:
  ModbusTCP(uint16_t serverPort = 502);
  ~ModbusTCP();

  void begin();
  void loop();
  void stop();
  bool isEnabled() {
    return enabled;
  }

  // Callback functions - to be implemented in main code
  bool (*readHoldingRegister)(uint16_t address, uint16_t* value) = nullptr;
  bool (*readInputRegister)(uint16_t address, uint16_t* value) = nullptr;
  bool (*writeHoldingRegister)(uint16_t address, uint16_t value) = nullptr;
};

#endif
