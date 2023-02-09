#ifndef _GROWATT_SPF_H_
#define _GROWATT_SPF_H_

#include "Arduino.h"
#include "GrowattTypes.h"

// Growatt modbus protocol version unknown from 2020-10-16
// From document: OffGrid SPF5000 Modbus RS485&RS232 RTU Protocol

typedef enum {
  SPF_I_STATUS = 0,
  SPF_PV1_V,
  SPF_PV2_V,
  SPF_PV1_CHGW,
  SPF_PV2_CHGW,
  SPF_BUCK1_I,
  SPF_BUCK2_I,
  SPF_OUT_PWR,
  SPF_OUT_VA,
  SPF_AC_CHGPWR,
  SPF_AC_CHGVA,
  SPF_BATT_V,
  SPF_BATT_SOC,
  SPF_BUS_V,
  SPF_GRID_V,
  SPF_LINE_F,
  SPF_OUT_V,
  SPF_OUT_F,
  SPF_OUT_DCV,
  SPF_INV_T,
  SPF_DCDC_T,
  SPF_LOAD,
  SPF_BUCK1_T,
  SPF_BUCK2_T,
  SPF_AC_INPWR,
  SPF_AC_INVA,
  SPF_BATT_PWR
} eSPFInputRegisters_t;

void init_growattSPF(sProtocolDefinition_t &Protocol);

#endif  // _GROWATT_SPF_H_
