/*
 * SerialLink.cpp
 * 
 * Implementierung der seriellen Kommunikation zwischen zwei ESP32
 */

#include "SerialLink.h"

SerialLink::SerialLink() 
  : _serial(&Serial2),
    _sentCount(0),
    _receivedCount(0),
    _errorCount(0) {
}

void SerialLink::begin(uint32_t baudrate, int8_t rxPin, int8_t txPin) {
  _serial->begin(baudrate, SERIAL_8N1, rxPin, txPin);
  _serial->setTimeout(SL_TIMEOUT_MS);
  
  // Buffer leeren
  while (_serial->available()) {
    _serial->read();
  }
  
  Serial.print(F("SerialLink initialized: "));
  Serial.print(baudrate);
  Serial.print(F(" baud, RX=GPIO"));
  Serial.print(rxPin);
  Serial.print(F(", TX=GPIO"));
  Serial.println(txPin);
  Serial.println(F("Note: Using GPIO4/27 (GPIO17 occupied by GPin_AL)"));
}

uint8_t SerialLink::calculateChecksum(const uint8_t* data, uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

bool SerialLink::waitForByte(uint32_t timeout_ms) {
  unsigned long start = millis();
  while (!_serial->available()) {
    if (millis() - start > timeout_ms) {
      return false;
    }
    yield();
  }
  return true;
}

bool SerialLink::sendMessage(uint8_t type, const uint8_t* data, uint8_t length) {
  if (length > SL_MAX_DATA_LENGTH) {
    _errorCount++;
    return false;
  }
  
  // Frame aufbauen
  uint8_t frame[SL_MAX_DATA_LENGTH + 6]; // START + TYPE + LENGTH + DATA + CHECKSUM + END
  uint8_t idx = 0;
  
  frame[idx++] = SL_START_BYTE;
  frame[idx++] = type;
  frame[idx++] = length;
  
  // Daten kopieren
  for (uint8_t i = 0; i < length; i++) {
    frame[idx++] = data[i];
  }
  
  // Checksum berechnen (TYPE + LENGTH + DATA)
  uint8_t checksum = calculateChecksum(&frame[1], length + 2);
  frame[idx++] = checksum;
  frame[idx++] = SL_END_BYTE;
  
  // Frame senden
  size_t written = _serial->write(frame, idx);
  _serial->flush();
  
  if (written == idx) {
    _sentCount++;
    return true;
  } else {
    _errorCount++;
    return false;
  }
}

bool SerialLink::sendWaterLevel(const WaterLevelData& data) {
  uint8_t buffer[sizeof(WaterLevelData)];
  memcpy(buffer, &data, sizeof(WaterLevelData));
  return sendMessage(SL_MSG_WATERLEVEL, buffer, sizeof(WaterLevelData));
}

bool SerialLink::sendPumpStatus(const PumpStatusData& data) {
  uint8_t buffer[sizeof(PumpStatusData)];
  memcpy(buffer, &data, sizeof(PumpStatusData));
  return sendMessage(SL_MSG_PUMP_STATUS, buffer, sizeof(PumpStatusData));
}

bool SerialLink::sendText(const char* text) {
  uint8_t len = strlen(text);
  if (len > SL_MAX_DATA_LENGTH) {
    len = SL_MAX_DATA_LENGTH;
  }
  return sendMessage(SL_MSG_TEXT, (const uint8_t*)text, len);
}

bool SerialLink::sendPing() {
  return sendMessage(SL_MSG_PING, nullptr, 0);
}

bool SerialLink::available() {
  // Prüfe ob START_BYTE im Buffer ist
  if (_serial->available() >= 4) { // Mindestens START + TYPE + LENGTH + END
    // Suche nach START_BYTE
    while (_serial->available()) {
      if (_serial->peek() == SL_START_BYTE) {
        return true;
      }
      _serial->read(); // Ungültiges Byte verwerfen
    }
  }
  return false;
}

bool SerialLink::receiveMessage(uint8_t& type, uint8_t* data, uint8_t& length) {
  // Warte auf START_BYTE
  if (!waitForByte(SL_TIMEOUT_MS)) {
    return false;
  }
  
  if (_serial->read() != SL_START_BYTE) {
    _errorCount++;
    return false;
  }
  
  // TYPE lesen
  if (!waitForByte(SL_TIMEOUT_MS)) {
    _errorCount++;
    return false;
  }
  type = _serial->read();
  
  // LENGTH lesen
  if (!waitForByte(SL_TIMEOUT_MS)) {
    _errorCount++;
    return false;
  }
  length = _serial->read();
  
  if (length > SL_MAX_DATA_LENGTH) {
    _errorCount++;
    return false;
  }
  
  // DATA lesen
  uint8_t frame[SL_MAX_DATA_LENGTH + 2]; // TYPE + LENGTH + DATA
  frame[0] = type;
  frame[1] = length;
  
  for (uint8_t i = 0; i < length; i++) {
    if (!waitForByte(SL_TIMEOUT_MS)) {
      _errorCount++;
      return false;
    }
    data[i] = _serial->read();
    frame[i + 2] = data[i];
  }
  
  // CHECKSUM lesen
  if (!waitForByte(SL_TIMEOUT_MS)) {
    _errorCount++;
    return false;
  }
  uint8_t receivedChecksum = _serial->read();
  
  // CHECKSUM prüfen
  uint8_t calculatedChecksum = calculateChecksum(frame, length + 2);
  if (receivedChecksum != calculatedChecksum) {
    _errorCount++;
    Serial.println(F("SerialLink: Checksum error!"));
    return false;
  }
  
  // END_BYTE lesen
  if (!waitForByte(SL_TIMEOUT_MS)) {
    _errorCount++;
    return false;
  }
  if (_serial->read() != SL_END_BYTE) {
    _errorCount++;
    return false;
  }
  
  _receivedCount++;
  return true;
}

void SerialLink::resetStatistics() {
  _sentCount = 0;
  _receivedCount = 0;
  _errorCount = 0;
}
