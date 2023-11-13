#pragma once
#include "GrowattTypes.h"
#include "Config.h"
#include <map>

class Growatt {
 public:
  Growatt();
  sProtocolDefinition_t _Protocol;
  using CommandHandlerFunc = std::function<std::tuple<bool, String>(
      const JsonDocument& req, JsonDocument& res, Growatt& inverter)>;

  void begin(Stream& serial);
  void InitProtocol();
  void RegisterCommand(const String& command, CommandHandlerFunc handler);
  void HandleCommand(const String& command, const byte* payload,
                     const unsigned int length, JsonDocument& res);
  bool ReadInputRegisters();
  bool ReadHoldingRegisters();
  bool ReadData();
  eDevice_t GetWiFiStickType();
  sGrowattModbusReg_t GetInputRegister(uint16_t reg);
  sGrowattModbusReg_t GetHoldingRegister(uint16_t reg);
  bool ReadInputReg(uint16_t adr, uint32_t* result);
  bool ReadInputReg(uint16_t adr, uint16_t* result);
  bool ReadHoldingReg(uint16_t adr, uint32_t* result);
  bool ReadHoldingReg(uint16_t adr, uint16_t* result);
  bool ReadHoldingRegFrag(uint16_t adr, uint8_t size, uint16_t* result);
  bool ReadHoldingRegFrag(uint16_t adr, uint8_t size, uint32_t* result);
  bool WriteHoldingReg(uint16_t adr, uint16_t value);
  bool WriteHoldingRegFrag(uint16_t adr, uint8_t size, uint16_t* value);
  void CreateJson(ShineJsonDocument& doc, String MacAddress);
  void CreateUIJson(ShineJsonDocument& doc);

 private:
  eDevice_t _eDevice;
  bool _GotData;
  uint32_t _PacketCnt;
  std::map<String, CommandHandlerFunc> handlers;

  eDevice_t _InitModbusCommunication();
  double roundByResolution(const double& value, const float& resolution);
  void JSONAddReg(sGrowattModbusReg_t* reg, JsonDocument& doc);
  std::tuple<bool, String> handleEcho(const JsonDocument& req,
                                      JsonDocument& res, Growatt& inverter);
  std::tuple<bool, String> handleCommandList(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter);
  std::tuple<bool, String> handleModbusGet(const JsonDocument& req,
                                           JsonDocument& res,
                                           Growatt& inverter);
  std::tuple<bool, String> handleModbusSet(const JsonDocument& req,
                                           JsonDocument& res,
                                           Growatt& inverter);
};
