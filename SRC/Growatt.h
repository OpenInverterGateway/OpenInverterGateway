#ifndef _GROWATT_H_
#define _GROWATT_H_

#include "Arduino.h"


typedef enum
{
  GwStatusWaiting = 0,
  GwStatusNormal  = 1,
  GwStatusFault   = 3
}eGrowattStatus_t;

typedef enum
{
  Undef_stick  = 0,
  ShineWiFi_S  = 1, // Serial
  ShineWiFi_X  = 2  // USB
}eDevice_t;

typedef struct
{
  eGrowattStatus_t  InverterStatus; 
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
  uint16_t u16Temperature;     // 0.1 degree celsius
  uint16_t u16PwrLimit;        // %
  uint16_t u16EnExportLimit;   // 0.1 %
  uint16_t u16ExportPwrLimit;  // 0.1 %
  uint16_t u16ExportFaultLimit;// 0.1 %
}sGrowattData_t;


class Growatt
{
  public:
    Growatt();
    void begin(Stream &serial);
    bool UpdateData();
    bool ReadHoldingReg(uint16_t adr, uint16_t* result);
    bool WriteHoldingReg(uint16_t adr, uint16_t value);

    eDevice_t        GetWiFiStickType();
    eGrowattStatus_t GetStatus();
    float            GetDcPower();
    float            GetDcVoltage();
    float            GetDcInputCurrent();
    float            GetAcFrequency();
    float            GetAcVoltage();
    float            GetAcOutputCurrent();
    float            GetAcPower();
    float            GetEnergyToday();
    float            GetEnergyTotal();
    uint32           GetOperatingTime();
    float            GetInverterTemperature();
    uint16_t         GetPwrLimit();
    uint16_t         GetEnExportLimit();
    uint16_t         GetExportPwrLimit();
    uint16_t         GetExportFaultLimit();    
  private:
    sGrowattData_t _Data;
    eDevice_t      _eDevice;

};

#endif // _GROWATT_H_
