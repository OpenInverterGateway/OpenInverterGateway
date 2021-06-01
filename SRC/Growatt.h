#ifndef _GROWATT_H_
#define _GROWATT_H_

#include "Arduino.h"


typedef enum
{
  GwStatusWaiting = 0,
  GwStatusNormal  = 1,
  GwStatusFault   = 2,
}eGrowattStatus;

typedef struct
{
  eGrowattStatus  InverterStatus; 
  uint32_t u32DcPower;         // in 0.1 W
  uint16_t u16DcVoltage;       // in 0.1 V
  uint16_t u16DcInputCurrent;  // in 0.1 A
  uint32_t u32AcPower;         // in 0.1 W
  uint16_t u16AcFrequency;     // in 0.01 Hz
  uint16_t u16AcVoltage;       // in 0.1 V
  uint16_t u16AcOutputCurrent; // in 0.1 A
  uint32_t u32EnergyToday;     // in 0.1 kWh
  uint32_t u32EnergyTotal;     // in 0.1 kWh
  uint32_t u32OperatingTime;   // in 0.5 s
  uint16_t u16Temperaure;      // 0.1 degree celsius
}GrowattData;


class Growatt
{
  public:
    Growatt();
    void begin(Stream &serial);
    bool UpdateData();

    eGrowattStatus GetStatus();
    float          GetDcPower();
    float          GetDcVoltage();
    float          GetDcInputCurrent();
    float          GetAcFrequency();
    float          GetAcVoltage();
    float          GetAcOutputCurrent();
    float          GetAcPower();
    float          GetEnergyToday();
    float          GetEnergyTotal();
    uint32         GetOperatingTime();
    float          GetInverterTemperature();
    
  private:
    GrowattData _Data;

};

#endif // _GROWATT_H_





