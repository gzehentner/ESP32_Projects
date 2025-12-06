/*
 * SerialLink.h
 * 
 * Einfache serielle Kommunikation zwischen zwei ESP32
 * Verwendet UART2 (Serial2) für die Verbindung
 * 
 * Hardware-Verbindung:
 * ESP32 #1 TX2 (GPIO 27) -> ESP32 #2 RX2 (GPIO 4)
 * ESP32 #1 RX2 (GPIO 4) -> ESP32 #2 TX2 (GPIO 27)
 * GND -> GND
 * 
 * Hinweis: GPIO 17 ist bei Wasserstand_V07 durch GPin_AL belegt
 * 
 * Protokoll:
 * - Frame: [START_BYTE][TYPE][LENGTH][DATA...][CHECKSUM][END_BYTE]
 * - START_BYTE: 0xAA
 * - END_BYTE: 0x55
 * - TYPE: 1 Byte (Nachrichtentyp)
 * - LENGTH: 1 Byte (Anzahl Datenbytes)
 * - DATA: 0-255 Bytes
 * - CHECKSUM: 1 Byte (XOR aller Bytes TYPE bis DATA)
 */

#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <Arduino.h>

// Protokoll-Konstanten
#define SL_START_BYTE 0xAA
#define SL_END_BYTE 0x55
#define SL_MAX_DATA_LENGTH 64
#define SL_TIMEOUT_MS 100

// Nachrichtentypen
enum SerialLinkMessageType {
  SL_MSG_PING = 0x01,           // Ping-Nachricht
  SL_MSG_PONG = 0x02,           // Pong-Antwort
  SL_MSG_WATERLEVEL = 0x10,     // Wasserstand-Daten
  SL_MSG_ALARM_STATE = 0x11,    // Alarmzustand
  SL_MSG_PUMP_STATUS = 0x12,    // Pumpenstatus
  SL_MSG_SENSOR_DATA = 0x13,    // Allgemeine Sensordaten
  SL_MSG_TEXT = 0x20,           // Textnachricht
  SL_MSG_ACK = 0xF0,            // Bestätigung
  SL_MSG_ERROR = 0xFF           // Fehler
};

// Struktur für Wasserstand-Daten
struct WaterLevelData {
  int16_t level_mm;             // Wasserstand in mm
  int16_t adc_value;            // ADC-Rohwert
  uint8_t alarm_state;          // Alarmzustand (0-6)
  uint32_t timestamp;           // Zeitstempel (seconds since startup)
};

// Struktur für Pumpenstatus
struct PumpStatusData {
  uint8_t pump_number;          // Pumpennummer (1-2)
  uint8_t is_running;           // Läuft (0/1)
  uint32_t runtime_seconds;     // Gesamtlaufzeit in Sekunden
  uint32_t runtime_total_hours; // Gesamtlaufzeit in Stunden
};

class SerialLink {
public:
  // Konstruktor
  SerialLink();
  
  // Initialisierung
  void begin(uint32_t baudrate = 9600, int8_t rxPin = 4, int8_t txPin = 27);
  
  // Nachrichten senden
  bool sendMessage(uint8_t type, const uint8_t* data, uint8_t length);
  bool sendWaterLevel(const WaterLevelData& data);
  bool sendPumpStatus(const PumpStatusData& data);
  bool sendText(const char* text);
  bool sendPing();
  
  // Nachrichten empfangen
  bool available();
  bool receiveMessage(uint8_t& type, uint8_t* data, uint8_t& length);
  
  // Statistik
  uint32_t getSentCount() { return _sentCount; }
  uint32_t getReceivedCount() { return _receivedCount; }
  uint32_t getErrorCount() { return _errorCount; }
  void resetStatistics();
  
private:
  HardwareSerial* _serial;
  uint32_t _sentCount;
  uint32_t _receivedCount;
  uint32_t _errorCount;
  
  // Hilfsfunktionen
  uint8_t calculateChecksum(const uint8_t* data, uint8_t length);
  bool waitForByte(uint32_t timeout_ms);
};

#endif // SERIALLINK_H
