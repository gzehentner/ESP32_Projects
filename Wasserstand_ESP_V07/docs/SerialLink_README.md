# SerialLink - Serielle Kommunikation zwischen zwei ESP32

## Überblick

SerialLink ist eine einfache Bibliothek zur seriellen Kommunikation zwischen zwei ESP32-Boards. Sie verwendet UART2 (Serial2) für die Datenübertragung, sodass der USB-Port (Serial) weiterhin für Debugging verfügbar bleibt.

## Hardware-Verbindung

Verbinde die beiden ESP32-Boards wie folgt:

```
ESP32 #1 (Sender)          ESP32 #2 (Empfänger)
-----------------          ---------------------
GPIO 27 (TX2) -----------> GPIO 4  (RX2)
GPIO 4  (RX2) <----------- GPIO 27 (TX2)
GND           -----------> GND
```

**Wichtig:** 
- Die ESP32-Boards müssen eine gemeinsame Masse (GND) haben
- TX des einen Boards geht zu RX des anderen Boards (gekreuzte Verbindung)
- Standard-Pins: RX=GPIO4, TX=GPIO27 (GPIO17 ist bei Wasserstand_V07 durch GPin_AL belegt)
- Diese Pins können bei Bedarf angepasst werden

## Protokoll

### Frame-Struktur

Jede Nachricht besteht aus folgenden Bytes:

```
[START][TYPE][LENGTH][DATA...][CHECKSUM][END]
```

- **START_BYTE**: 0xAA (Marker für Nachrichtenbeginn)
- **TYPE**: 1 Byte (Nachrichtentyp, siehe unten)
- **LENGTH**: 1 Byte (Anzahl der Datenbytes, 0-64)
- **DATA**: 0-64 Bytes (Nutzdaten)
- **CHECKSUM**: 1 Byte (XOR aller Bytes TYPE bis DATA)
- **END_BYTE**: 0x55 (Marker für Nachrichtenende)

### Nachrichtentypen

| Type | Wert | Beschreibung |
|------|------|--------------|
| SL_MSG_PING | 0x01 | Ping-Nachricht (Keep-alive) |
| SL_MSG_PONG | 0x02 | Antwort auf Ping |
| SL_MSG_WATERLEVEL | 0x10 | Wasserstand-Daten |
| SL_MSG_ALARM_STATE | 0x11 | Alarmzustand |
| SL_MSG_PUMP_STATUS | 0x12 | Pumpenstatus |
| SL_MSG_SENSOR_DATA | 0x13 | Allgemeine Sensordaten |
| SL_MSG_TEXT | 0x20 | Textnachricht (null-terminiert) |
| SL_MSG_ACK | 0xF0 | Bestätigung |
| SL_MSG_ERROR | 0xFF | Fehlermeldung |

## Verwendung im Code

### Initialisierung

```cpp
#include <SerialLink.h>

SerialLink serialLink;

void setup() {
  Serial.begin(115200);
  
  // SerialLink initialisieren
  // Parameter: Baudrate, RX-Pin, TX-Pin
  serialLink.begin(9600, 16, 17);
}
```

### Daten senden

#### Wasserstand senden

```cpp
WaterLevelData data;
data.level_mm = 1250;           // Wasserstand in mm
data.adc_value = 2048;          // ADC-Rohwert
data.alarm_state = 2;           // Alarmzustand (0-6)
data.timestamp = millis()/1000; // Zeitstempel

serialLink.sendWaterLevel(data);
```

#### Pumpenstatus senden

```cpp
PumpStatusData pumpData;
pumpData.pump_number = 1;           // Pumpennummer
pumpData.is_running = 1;            // 1=läuft, 0=aus
pumpData.runtime_seconds = 3600;    // Laufzeit
pumpData.runtime_total_hours = 100; // Gesamtlaufzeit

serialLink.sendPumpStatus(pumpData);
```

#### Textnachricht senden

```cpp
serialLink.sendText("Hello from ESP32!");
```

#### Ping senden

```cpp
serialLink.sendPing();
```

#### Benutzerdefinierte Nachricht senden

```cpp
uint8_t data[] = {0x12, 0x34, 0x56};
serialLink.sendMessage(SL_MSG_SENSOR_DATA, data, 3);
```

### Daten empfangen

```cpp
void loop() {
  // Prüfe ob Daten verfügbar sind
  if (serialLink.available()) {
    uint8_t msgType;
    uint8_t msgData[SL_MAX_DATA_LENGTH];
    uint8_t msgLength;
    
    // Nachricht empfangen
    if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
      
      // Nachricht verarbeiten
      switch (msgType) {
        case SL_MSG_WATERLEVEL:
          if (msgLength == sizeof(WaterLevelData)) {
            WaterLevelData wlData;
            memcpy(&wlData, msgData, sizeof(WaterLevelData));
            
            Serial.print("Water Level: ");
            Serial.print(wlData.level_mm);
            Serial.println(" mm");
          }
          break;
          
        case SL_MSG_TEXT:
          msgData[msgLength] = '\0'; // Null-Terminierung
          Serial.print("Text: ");
          Serial.println((char*)msgData);
          break;
          
        case SL_MSG_PING:
          // Auf Ping mit Pong antworten
          serialLink.sendMessage(SL_MSG_PONG, nullptr, 0);
          break;
      }
    }
  }
}
```

### Statistik abrufen

```cpp
Serial.print("Sent: ");
Serial.println(serialLink.getSentCount());

Serial.print("Received: ");
Serial.println(serialLink.getReceivedCount());

Serial.print("Errors: ");
Serial.println(serialLink.getErrorCount());

// Statistik zurücksetzen
serialLink.resetStatistics();
```

## Integration in Wasserstand_V7

Die SerialLink-Bibliothek ist bereits in das Wasserstand_V7-Projekt integriert:

1. **Automatische Initialisierung** in `setup()`:
   - SerialLink wird mit 9600 Baud auf GPIO16/17 initialisiert

2. **Periodisches Senden** in `loop()`:
   - Alle 5 Sekunden werden Wasserstand-Daten gesendet
   - Daten enthalten: Wasserstand, ADC-Wert, Alarmzustand, Zeitstempel

3. **Empfang** in `loop()`:
   - Eingehende Nachrichten werden automatisch verarbeitet
   - PING wird mit PONG beantwortet
   - Empfangene Daten werden auf Serial ausgegeben

4. **Statistik**:
   - Alle 60 Sekunden wird eine Statistik ausgegeben

## Beispiel-Projekt für zweites ESP32

Ein vollständiges Beispiel-Projekt für das empfangende ESP32-Board befindet sich in:
```
examples/SerialLink_Receiver_Example.ino
```

Das Beispiel zeigt:
- Empfang und Verarbeitung aller Nachrichtentypen
- Automatische Antwort auf PING
- Periodisches Senden von PING
- Statistik-Ausgabe

## Anpassung der Pins

Falls andere Pins verwendet werden sollen:

```cpp
// Andere Pins verwenden
serialLink.begin(9600, 25, 26); // RX=GPIO25, TX=GPIO26
```

**Hinweis:** Nicht alle GPIO-Pins sind für alle Funktionen geeignet. Die gewählten Pins GPIO4 und GPIO27 sind gut für UART2 geeignet und kollidieren nicht mit den verwendeten Pins des Wasserstand-Systems (GPIO17 ist dort durch GPin_AL belegt).

## Anpassung der Baudrate

Für höhere Datenraten (bei gutem Signal und kurzen Kabeln):

```cpp
serialLink.begin(115200, 16, 17); // 115200 Baud
```

Für sehr lange Kabel oder gestörte Umgebung:

```cpp
serialLink.begin(4800, 16, 17); // 4800 Baud (robuster)
```

## Fehlerbehandlung

Die Bibliothek prüft automatisch:
- Start- und End-Marker
- Checksumme
- Timeouts beim Empfang
- Maximale Nachrichtenlänge

Fehler werden gezählt und können über `getErrorCount()` abgefragt werden.

## Troubleshooting

### Keine Kommunikation

1. **Verkabelung prüfen:**
   - TX -> RX gekreuzt?
   - GND verbunden?
   - Richtige GPIOs?

2. **Baudrate prüfen:**
   - Gleiche Baudrate auf beiden Seiten?

3. **Pins prüfen:**
   - Pins nicht doppelt verwendet?
   - Pins Hardware-UART-fähig?

### Viele Fehler

1. **Kabel zu lang:**
   - Baudrate reduzieren
   - Geschirmtes Kabel verwenden

2. **Störungen:**
   - Kabel von Stromversorgung fernhalten
   - Ferrite-Beads verwenden

3. **Timing-Probleme:**
   - Timeout-Wert erhöhen (in SerialLink.h)

## Performance

- **Baudrate:** 9600 Baud (Standard) = ca. 960 Bytes/Sekunde
- **Overhead:** 6 Bytes pro Nachricht (Start, Type, Length, Checksum, End)
- **Maximale Nutzdaten:** 64 Bytes pro Nachricht
- **Typische Latenz:** < 100 ms

Bei 9600 Baud und 5-Sekunden-Intervall werden ca. 14 Bytes pro Nachricht übertragen, was zu einer sehr geringen Bus-Auslastung von unter 1% führt.

## Lizenz

Dieses Projekt steht unter der gleichen Lizenz wie das Wasserstand_V7-Projekt.

## Autor

Georg Zehentner
