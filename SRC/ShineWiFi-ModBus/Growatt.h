#ifndef _GROWATT_H_
#define _GROWATT_H_

#include "GrowattTypes.h"

class Growatt
{
  public:
    Growatt();
    sProtocolDefinition_t _Protocol;

    void begin(Stream& serial);
    void InitProtocol();

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
    bool WriteHoldingReg(uint16_t adr, uint16_t value);
    void CreateJson(char* Buffer, const char* MacAddress);
    void CreateUIJson(char* Buffer);

  private:
    eDevice_t _eDevice;
    bool _GotData;
    uint32_t _PacketCnt;

    eDevice_t _InitModbusCommunication();
    static double _round2(double value);
};

#endif // _GROWATT_H_
