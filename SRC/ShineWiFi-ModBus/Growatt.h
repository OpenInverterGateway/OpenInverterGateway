#ifndef _GROWATT_H_
#define _GROWATT_H_

#include "Arduino.h"
#include "GrowattTypes.h"

class Growatt {
  public:
    Growatt();
    sProtocolDefinition_t _Protocol;

    void begin(Stream &serial);
    void InitProtocol(uint16_t version);
    bool ReadData();
    eDevice_t GetWiFiStickType();
    sGrowattModbusReg_t GetInputRegister(SupportedModbusInputRegisters_t register);
    sGrowattModbusReg_t GetHoldingRegister(SupportedModbusHoldingRegisters_t register);
    bool ReadInputReg(uint16_t adr, uint32_t* result);
    bool ReadInputReg(uint16_t adr, uint16_t* result);
    bool ReadHoldingReg(uint16_t adr, uint32_t* result);
    bool ReadHoldingReg(uint16_t adr, uint16_t* result);
    bool WriteHoldingReg(uint16_t adr, uint16_t value);

    void CreateJson(char *Buffer, const char *MacAddress);
    void CreateUIJson(char *Buffer);

    eGrowattStatus_t GetStatus();
    float GetAcPower();

  private:
    eDevice_t _eDevice;
    bool _GotData;
    uint32_t _PacketCnt;

    eDevice_t _InitModbusCommunication();
};

#endif // _GROWATT_H_