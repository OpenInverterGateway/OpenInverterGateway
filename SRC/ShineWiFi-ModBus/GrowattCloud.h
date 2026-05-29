/**
 * GrowattCloud.h - Growatt Cloud Protocol Sender for OpenInverterGateway
 *
 * Implements the Growatt proprietary TCP protocol (port 5279) to send
 * inverter data to server.growatt.com while simultaneously publishing
 * to MQTT/Prometheus locally. This enables dual-mode operation:
 * local monitoring + Growatt app/cloud access.
 *
 * Protocol details reverse-engineered from stock ShineWiFi-S firmware
 * and community documentation (Grott, nwf, sciurius).
 *
 * Author: OpenInverterGateway contributors
 * Date: 2026-05-19
 * License: MIT
 */

#ifndef GROWATT_CLOUD_H
#define GROWATT_CLOUD_H

#include <Arduino.h>
#include <WiFiClient.h>

// --- Protocol Constants ---
#define GROWATT_SERVER_DEFAULT "server.growatt.com"
#define GROWATT_PORT_DEFAULT 5279

// Protocol IDs (bytes 2-3 of header)
#define GROWATT_PROTO_PLAIN 0x0000   // Modbus standard, no encryption
#define GROWATT_PROTO_V3 0x0002      // Growatt v3.x, no encryption
#define GROWATT_PROTO_ENC_V1 0x0005  // Growatt encrypted v1 (XOR)
#define GROWATT_PROTO_ENC_V2 0x0006  // Growatt encrypted v2 (XOR)

// Function codes (byte 7 of header)
#define GROWATT_FUNC_WRITE_REG 0x06   // Write single holding register (Modbus FC06)
#define GROWATT_FUNC_ANNOUNCE 0x03    // DATA3: device announcement
#define GROWATT_FUNC_DATA 0x04        // DATA4: periodic telemetry
#define GROWATT_FUNC_PING 0x16        // Keepalive heartbeat
#define GROWATT_FUNC_CONFIGURE 0x18   // Server pushes config
#define GROWATT_FUNC_IDENTIFY 0x19    // Server queries config
#define GROWATT_FUNC_SMARTMETER 0x20  // Smart meter data

// Identify config items (for responding to server queries)
#define GROWATT_CFG_INTERVAL 0x04       // Log interval (minutes)
#define GROWATT_CFG_ADDR_MIN 0x05       // Modbus address range min
#define GROWATT_CFG_ADDR_MAX 0x06       // Modbus address range max
#define GROWATT_CFG_DATALOGGER_ID 0x08  // Datalogger serial
#define GROWATT_CFG_LOCAL_IP 0x0E       // Local IP address
#define GROWATT_CFG_LOCAL_PORT 0x0F     // Local port
#define GROWATT_CFG_MAC 0x10            // MAC address
#define GROWATT_CFG_SERVER_LEN 0x11     // Server hostname length
#define GROWATT_CFG_SERVER_PORT 0x12    // Server port
#define GROWATT_CFG_SERVER_ADDR 0x13    // Server address (ENFORCED!)
#define GROWATT_CFG_FW_VERSION 0x15     // Firmware version string

// Timing (milliseconds)
#define GROWATT_PING_INTERVAL 180000   // 3 minutes
#define GROWATT_DATA_INTERVAL 300000   // 5 minutes (configurable)
#define GROWATT_ANNOUNCE_RETRY 30000   // 30 seconds until ACKed
#define GROWATT_CONNECT_TIMEOUT 10000  // TCP connect timeout
#define GROWATT_READ_TIMEOUT 5000      // TCP read timeout
#define GROWATT_MAX_UNACKED 15         // Drop connection after N unacked

// Buffer sizes
#define GROWATT_SERIAL_LEN 30    // Serial field width (NUL-padded)
#define GROWATT_HEADER_LEN 8     // Protocol header size
#define GROWATT_MAX_PACKET 512   // Max packet size
#define GROWATT_TIMESTAMP_LEN 6  // 6 single bytes: Y-2000, M, D, H, M, S

// XOR encryption mask
static const uint8_t GROWATT_XOR_MASK[] = {0x47, 0x72, 0x6F, 0x77,
                                           0x61, 0x74, 0x74};
#define GROWATT_XOR_MASK_LEN 7

// --- Structures ---

struct GrowattCloudConfig {
  char serverHost[64];
  uint16_t serverPort;
  char dataloggerSerial[GROWATT_SERIAL_LEN + 1];  // Read from config portal
  char inverterSerial[GROWATT_SERIAL_LEN + 1];  // Read from inverter via Modbus
  char fwVersion[16];
  uint8_t logIntervalMin;  // Data report interval in minutes
  uint16_t protocolId;     // 0x0005 for encrypted v1
  bool enabled;            // Enable/disable cloud sending
};

// Connection state machine
enum GrowattCloudState {
  GCS_DISCONNECTED,
  GCS_CONNECTING,
  GCS_CONNECTED,
  GCS_ANNOUNCED,  // ANNOUNCE sent, waiting for ACK
  GCS_ACTIVE,     // ACK received, sending periodic DATA
  GCS_ERROR
};

class GrowattCloud {
 public:
  GrowattCloud();

  /**
   * Initialize with configuration.
   * Call once in setup() after reading config from portal/EEPROM.
   */
  void begin(const GrowattCloudConfig& config);

  /**
   * Call in loop(). Handles connection, ping, announce, data sending.
   * Pass current inverter register data for periodic reports.
   *
   * @param inputRegs   Array of input register values (FC04, from inverter)
   * @param numInputRegs Number of input registers
   * @param holdingRegs  Array of holding register values (FC03, from inverter)
   * @param numHoldingRegs Number of holding registers
   */
  void loop(const uint16_t* inputRegs, uint16_t numInputRegs,
            const uint16_t* holdingRegs = nullptr, uint16_t numHoldingRegs = 0);

  /**
   * Check if connected and active.
   */
  bool isConnected() const {
    return _state == GCS_ACTIVE || _state == GCS_ANNOUNCED;
  }

  /**
   * Get current state for debugging.
   */
  GrowattCloudState getState() const { return _state; }

  /**
   * Force reconnection.
   */
  void reconnect();

  /**
   * Update inverter serial (discovered via Modbus after boot).
   */
  void setInverterSerial(const char* serial);

  /**
   * Get stats for web UI / debugging.
   */
  uint32_t getPacketsSent() const { return _packetsSent; }
  uint32_t getPacketsRecv() const { return _packetsRecv; }
  uint32_t getReconnects() const { return _reconnects; }
  uint32_t getLastSendTime() const { return _lastDataSend; }

 private:
  // --- Connection management ---
  void _connect();
  void _disconnect();
  bool _checkConnection();

  // --- Protocol message builders ---
  uint16_t _buildPing(uint8_t* buf);
  uint16_t _buildAnnounce(uint8_t* buf, const uint16_t* holdingRegs,
                          uint16_t numHoldingRegs);
  uint16_t _buildData(uint8_t* buf, const uint16_t* inputRegs,
                      uint16_t numInputRegs);

  // --- Protocol message handlers ---
  void _handleReceived();
  void _handleAck(uint8_t funcCode);
  void _handleIdentify(const uint8_t* data, uint16_t len);
  void _handleConfigure(const uint8_t* data, uint16_t len);
  void _handleWriteReg(const uint8_t* data, uint16_t len);

  // --- Header and encryption ---
  void _writeHeader(uint8_t* buf, uint16_t transId, uint16_t protoId,
                    uint16_t dataLen, uint8_t unitId, uint8_t funcCode);
  void _xorEncrypt(uint8_t* data, uint16_t offset, uint16_t len);
  void _xorDecrypt(uint8_t* data, uint16_t offset, uint16_t len);
  uint16_t _calcCRC16(const uint8_t* data, uint16_t len);
  void _writeSerial(uint8_t* buf, const char* serial);
  void _writeTimestamp(uint8_t* buf);

  // --- Send/receive helpers ---
  bool _sendPacket(uint8_t* buf, uint16_t len);
  int _readPacket(uint8_t* buf, uint16_t maxLen);

  // --- State ---
  WiFiClient _client;
  GrowattCloudConfig _config;
  GrowattCloudState _state;
  uint16_t _transactionId;
  uint32_t _lastPing;
  uint32_t _lastDataSend;
  uint32_t _lastAnnounce;
  uint32_t _lastConnectAttempt;
  uint8_t _unackedCount;
  bool _announceAcked;

  // --- Stats ---
  uint32_t _packetsSent;
  uint32_t _packetsRecv;
  uint32_t _reconnects;

  // --- Buffers ---
  uint8_t _txBuf[GROWATT_MAX_PACKET];
  uint8_t _rxBuf[GROWATT_MAX_PACKET];
};

#endif  // GROWATT_CLOUD_H
