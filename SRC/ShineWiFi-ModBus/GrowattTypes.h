#pragma once
#include "Arduino.h"
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define JSON_DOCUMENT_SIZE 4096
#define BUFFER_SIZE 256

typedef enum {
  Undef_stick = 0,
  ShineWiFi_S = 1,  // Serial DB9-Connector, 9600Bd, Protocol v3.05 (2013)
  ShineWiFi_X =
      2,  // USB Type A with Bajonet locking , 115200Bd, Protocol v1.05 (2018)
  ShineWiFi_F = 3  // USB Type A DB9-style screws, (Baudrate and protocol
                   // unclear; likely 115200Bd / v1.05)
} eDevice_t;

typedef enum {
  GwStatusWaiting,
  GwStatusNormal,
  GwStatusFault
} eGrowattStatus_t;

// Protocol v3.05 also knows Autostart, v1.05 does not:
typedef enum {
  GwOnOffOFF = 0x0000,
  GwOnOffON = 0x0001,  // default for v1.05
  GwOnOffOFFAuto = 0x0100,
  GwOnOffONAuto = 0x0101  // default for v3.05
} eGrowattOnOff_t;

typedef enum {
  H_STATUS = 0,
} SupportedModbusHoldingRegisters_t;

typedef enum {
  NONE,
  POWER_W,
  POWER_KWH,
  VOLTAGE,
  CURRENT,
  SECONDS,
  PERCENTAGE,
  FREQUENCY,
  TEMPERATURE,
  VA,
  CURRENT_M,
  RESISTANCE_K,
  POWER_REACTIVE,
} RegisterUnit_t;

typedef enum {
  SIZE_16BIT,
  SIZE_32BIT,
  SIZE_16BIT_S,
  SIZE_32BIT_S,
} RegisterSize_t;

typedef struct {
  uint16_t address;
  uint32_t value;
  RegisterSize_t size;
  const __FlashStringHelper* name;
  float multiplier;
  float resolution;
  RegisterUnit_t unit;
  bool frontend;
  bool plot;
} sGrowattModbusReg_t;

// Growatt limits maximal number of registers that can be polled
// with a single read. Define reading frames using this. Can be nicer..
typedef struct {
  uint16_t StartAddress;
  uint8_t FragmentSize;
} sGrowattReadFragment_t;

typedef struct {
  uint16_t InputRegisterCount;
  uint8_t InputFragmentCount;
  uint16_t HoldingRegisterCount;
  uint8_t HoldingFragmentCount;
  sGrowattModbusReg_t InputRegisters[125];
  sGrowattModbusReg_t HoldingRegisters[10];
  sGrowattReadFragment_t InputReadFragments[10];
  sGrowattReadFragment_t HoldingReadFragments[10];
} sProtocolDefinition_t;
