/*
 * SerialLink_Receiver_Example.cpp
 * 
 * Beispiel-Programm für das zweite ESP32 Board (PlatformIO)
 * Optimiert für ESP32-CAM WROVER Board
 * Empfängt Daten vom Wasserstand-System via SerialLink
 * 
 * Hardware-Verbindung für ESP32-CAM:
 * ESP32 Sender TX2 (GPIO 27) -> ESP32-CAM RX (GPIO 14)
 * ESP32 Sender RX2 (GPIO 4)  -> ESP32-CAM TX (GPIO 13)
 * GND                        -> GND
 * 
 * Hinweise für ESP32-CAM:
 * - GPIO 0-1, 3: Für USB-Serial reserviert (vermeiden für UART2)
 * - GPIO 14: MTMS, RX für SerialLink (nicht boot-sensitiv!)
 * - GPIO 13: MTCK, TX für SerialLink
 * - Viele Pins durch Kamera belegt (siehe main.cpp)
 * 
 * Verwendung in PlatformIO:
 * 1. Kopiere diese Datei als main.cpp in das ESP32-CAM Projekt
 * 2. Kopiere SerialLink.h in das include/ Verzeichnis
 * 3. Kopiere SerialLink.cpp in das src/ Verzeichnis
 * 4. Board-Einstellung: board = esp32cam in platformio.ini
 * 5. Kompiliere und lade das Programm auf ESP32-CAM
 * 6. Öffne den Serial Monitor mit 115200 Baud
 * 
 * WICHTIG für Upload:
 * - GPIO 0 mit GND verbinden zum Flashen
 * - Nach Upload: GPIO 0 trennen und Reset drücken
 */

#include <Arduino.h>
#include <SerialLink.h>

// SerialLink-Objekt erstellen
SerialLink serialLink;

// Letzte empfangene Daten
int lastWaterLevel_mm = 0;
int lastAlarmState = 0;
unsigned long lastDataReceived = 0;

void setup() {
  // USB-Serial für Debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n\n================================="));
  Serial.println(F("SerialLink Receiver ESP32-CAM"));
  Serial.println(F("=================================\n"));
  
  // SerialLink initialisieren für ESP32-CAM
  // Baudrate: 9600, RX=GPIO14, TX=GPIO13
  // Diese Pins sind auf ESP32-CAM frei (Kamera nutzt sie nicht)
  // GPIO 14 ist nicht boot-sensitiv (besser als GPIO 12)
  // Verkabelung: Sender TX2(GPIO27) -> Empfänger RX(GPIO14)
  //              Sender RX2(GPIO4)  -> Empfänger TX(GPIO13)
  serialLink.begin(9600, 14, 13);
  
  Serial.println(F("SerialLink bereit:"));
  Serial.println(F("  RX: GPIO 14"));
  Serial.println(F("  TX: GPIO 13"));
  Serial.println(F("\nWaiting for data from sender...\n"));
}

void loop() {
  // Prüfe auf eingehende Nachrichten
  if (serialLink.available()) {
    uint8_t msgType;
    uint8_t msgData[SL_MAX_DATA_LENGTH];
    uint8_t msgLength;
    
    if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
      lastDataReceived = millis();
      
      // Nachrichtentyp ausgeben
      Serial.print(F("Received message: Type=0x"));
      Serial.print(msgType, HEX);
      Serial.print(F(", Length="));
      Serial.println(msgLength);
      
      // Nachricht verarbeiten
      switch (msgType) {
        case SL_MSG_PING:
          Serial.println(F("  -> PING received, sending PONG"));
          serialLink.sendMessage(SL_MSG_PONG, nullptr, 0);
          break;
          
        case SL_MSG_PONG:
          Serial.println(F("  -> PONG received"));
          break;
          
        case SL_MSG_TEXT:
          msgData[msgLength] = '\0'; // Null-Terminierung
          Serial.print(F("  -> Text: "));
          Serial.println((char*)msgData);
          break;
          
        case SL_MSG_WATERLEVEL:
          if (msgLength == sizeof(WaterLevelData)) {
            WaterLevelData data;
            memcpy(&data, msgData, sizeof(WaterLevelData));
            
            lastWaterLevel_mm = data.level_mm;
            lastAlarmState = data.alarm_state;
            
            Serial.println(F("  -> Water Level Data:"));
            Serial.print(F("     Level: "));
            Serial.print(data.level_mm);
            Serial.println(F(" mm"));
            Serial.print(F("     ADC: "));
            Serial.println(data.adc_value);
            Serial.print(F("     Alarm State: "));
            Serial.println(data.alarm_state);
            Serial.print(F("     Timestamp: "));
            Serial.print(data.timestamp);
            Serial.println(F(" seconds"));
            
            // Beispiel: Aktion bei Alarm ausführen
            if (data.alarm_state >= 4) {
              Serial.println(F("     >>> WARNING: High alarm state! <<<"));
            }
          }
          break;
          
        case SL_MSG_PUMP_STATUS:
          if (msgLength == sizeof(PumpStatusData)) {
            PumpStatusData data;
            memcpy(&data, msgData, sizeof(PumpStatusData));
            
            Serial.println(F("  -> Pump Status Data:"));
            Serial.print(F("     Pump: "));
            Serial.println(data.pump_number);
            Serial.print(F("     Running: "));
            Serial.println(data.is_running ? "YES" : "NO");
            Serial.print(F("     Runtime: "));
            Serial.print(data.runtime_seconds);
            Serial.println(F(" seconds"));
            Serial.print(F("     Total: "));
            Serial.print(data.runtime_total_hours);
            Serial.println(F(" hours"));
          }
          break;
          
        case SL_MSG_ALARM_STATE:
          if (msgLength >= 1) {
            Serial.print(F("  -> Alarm State: "));
            Serial.println(msgData[0]);
          }
          break;
          
        default:
          Serial.println(F("  -> Unknown message type"));
          // Optional: Rohdaten ausgeben
          Serial.print(F("     Data: "));
          for (uint8_t i = 0; i < msgLength; i++) {
            Serial.print(msgData[i], HEX);
            Serial.print(F(" "));
          }
          Serial.println();
          break;
      }
      
      Serial.println(); // Leerzeile nach jeder Nachricht
    }
  }
  
  // Alle 10 Sekunden einen Ping senden (optional)
  static unsigned long lastPing = 0;
  if (millis() - lastPing > 10000) {
    lastPing = millis();
    Serial.println(F("Sending PING..."));
    serialLink.sendPing();
  }
  
  // Alle 60 Sekunden Statistik ausgeben
  static unsigned long lastStats = 0;
  if (millis() - lastStats > 60000) {
    lastStats = millis();
    
    Serial.println(F("\n=== Statistics ==="));
    Serial.print(F("Messages sent: "));
    Serial.println(serialLink.getSentCount());
    Serial.print(F("Messages received: "));
    Serial.println(serialLink.getReceivedCount());
    Serial.print(F("Errors: "));
    Serial.println(serialLink.getErrorCount());
    
    if (lastDataReceived > 0) {
      Serial.print(F("Last data received: "));
      Serial.print((millis() - lastDataReceived) / 1000);
      Serial.println(F(" seconds ago"));
      Serial.print(F("Last water level: "));
      Serial.print(lastWaterLevel_mm);
      Serial.println(F(" mm"));
      Serial.print(F("Last alarm state: "));
      Serial.println(lastAlarmState);
    } else {
      Serial.println(F("No data received yet"));
    }
    Serial.println(F("==================\n"));
  }
  
  // Kurze Pause
  delay(10);
}
