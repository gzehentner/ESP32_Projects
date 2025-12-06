# SerialLink auf ESP32-CAM WROVER

## Überblick
Diese Anleitung beschreibt die Integration von SerialLink auf einem ESP32-CAM WROVER Board als Empfänger für Wasserstand-Daten.

## Hardware-Spezifikation ESP32-CAM

### Von der Kamera belegte Pins
Die Kamera-Schnittstelle belegt folgende GPIOs:
- **GPIO 0**: XCLK (Camera Clock)
- **GPIO 5**: VSYNC
- **GPIO 18**: Y3_GPIO
- **GPIO 19**: Y4_GPIO
- **GPIO 21**: Y5_GPIO
- **GPIO 22**: PCLK
- **GPIO 23**: HREF
- **GPIO 25**: Y9_GPIO
- **GPIO 26**: SIOD (I2C Data für Kamera)
- **GPIO 27**: SIOC (I2C Clock für Kamera) ⚠️
- **GPIO 32**: PWDN (Power Down)
- **GPIO 34, 35, 36, 39**: Y8, Y7, Y6, Y2

### Verfügbare Pins für SerialLink
Da GPIO 27 von der Kamera-I2C belegt ist, verwenden wir:
- **GPIO 14** (RX) - MTMS (nicht boot-sensitiv!)
- **GPIO 13** (TX) - MTCK

**Vorteil:** GPIO 14 ist nicht boot-sensitiv, im Gegensatz zu GPIO 12 (MTDI)

## Verkabelung

### Verbindung Wasserstand-ESP32 ↔ ESP32-CAM

```
Wasserstand ESP32          ESP32-CAM WROVER
-----------------          -----------------
GPIO 27 (TX2)     ----->   GPIO 14 (RX)
GPIO 4  (RX2)     <-----   GPIO 13 (TX)
GND               ------   GND
```

### Spannungsversorgung
- ESP32-CAM benötigt 5V mit min. 500mA (Kamera!)
- Über USB-Adapter oder externes Netzteil
- **Nicht** direkt von anderem ESP32 versorgen

## Software-Integration

### 1. Projektstruktur
```
ESP32 Cam Take Photo/
├── platformio.ini
├── include/
│   ├── SerialLink.h      <- Kopieren aus Wasserstand-Projekt
│   └── ... (bestehende Header)
└── src/
    ├── SerialLink.cpp    <- Kopieren aus Wasserstand-Projekt
    ├── main.cpp          <- Anpassen für SerialLink
    └── ... (bestehende Sourcen)
```

### 2. platformio.ini anpassen
```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
build_flags = 
    -DCORE_DEBUG_LEVEL=0
lib_deps = 
    # Bestehende Abhängigkeiten
```

### 3. Code-Integration in main.cpp

```cpp
#include <SerialLink.h>

SerialLink serialLink;

void setup() {
    Serial.begin(115200);
    
    // Kamera-Initialisierung (bestehend)
    // ...
    
    // SerialLink initialisieren
    serialLink.begin(9600, 14, 13);  // RX=14, TX=13
    Serial.println("SerialLink bereit auf GPIO 14/13");
}

void loop() {
    // Bestehender Kamera-Code
    // ...
    
    // SerialLink-Daten empfangen
    if (serialLink.available()) {
        uint8_t msgType;
        uint8_t msgData[64];
        uint8_t msgLength;
        
        if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
            switch(msgType) {
                case SL_MSG_WATERLEVEL:
                    WaterLevelData* wlData = (WaterLevelData*)msgData;
                    Serial.printf("Wasserstand: %d mm\n", wlData->level_mm);
                    // Hier weitere Verarbeitung, z.B. auf Webseite anzeigen
                    break;
                    
                case SL_MSG_ALARM_STATE:
                    uint8_t alarmState = msgData[0];
                    Serial.printf("Alarm: %d\n", alarmState);
                    break;
            }
        }
    }
}
```

### 4. Alternative: Standalone Receiver
Für Tests ohne Kamera kann `SerialLink_Receiver_Example.cpp` als `main.cpp` verwendet werden:

```bash
# Im Wasserstand-Projekt:
cp examples/SerialLink_Receiver_Example.cpp ../ESP32\ Cam\ Take\ Photo/src/main.cpp

# Dateien kopieren:
cp include/SerialLink.h ../ESP32\ Cam\ Take\ Photo/include/
cp src/SerialLink.cpp ../ESP32\ Cam\ Take\ Photo/src/
```

## Upload und Debugging

### ESP32-CAM in Programmiermodus
1. GPIO 0 mit GND verbinden (vor Einstecken!)
2. USB-Adapter anschließen
3. `pio run --target upload` ausführen
4. Nach Upload: GPIO 0 trennen
5. Reset-Button drücken

### Serial Monitor
```bash
pio device monitor --baud 115200
```

**Erwartete Ausgabe:**
```
=================================
SerialLink Receiver ESP32-CAM
=================================

SerialLink bereit:
  RX: GPIO 12
  TX: GPIO 13

Waiting for data from sender...

[2024-01-15 10:30:45] Received WATERLEVEL: 1234 mm, ADC: 2048
[2024-01-15 10:30:50] Received WATERLEVEL: 1235 mm, ADC: 2049
```

## Troubleshooting

### Problem: Keine Daten empfangen
**Diagnose:**
```cpp
// In loop() temporär hinzufügen:
Serial.printf("Serial2 available: %d\n", Serial2.available());
```

**Lösungen:**
- Verkabelung prüfen (TX↔RX gekreuzt?)
- GND-Verbindung prüfen
- Baudrate auf beiden Seiten gleich (9600)?
- Mit Logic Analyzer oder Oszilloskop Signale prüfen

### Problem: ESP32-CAM startet nicht
**Hinweis:** Mit GPIO 14 sollte dieses Problem nicht auftreten, da GPIO 14 nicht boot-sensitiv ist.

Falls doch Probleme beim Boot:
```cpp
// In setup() vor serialLink.begin():
pinMode(14, INPUT_PULLDOWN);  // Sicherheits-Pull-Down
delay(100);
serialLink.begin(9600, 14, 13);
```

### Problem: Kamera funktioniert nicht mehr
**Ursache:** Pin-Konflikt oder Stromversorgung

**Prüfen:**
1. Keine Verkabelung auf GPIO 26/27 (Kamera I2C!)
2. Netzteil: min. 5V/500mA
3. Kamera-Init vor SerialLink-Init

### Problem: Upload schlägt fehl
**Lösungen:**
- GPIO 0 sicher mit GND verbinden
- USB-Adapter Treiber prüfen (CH340/CP2102)
- Niedrigere Upload-Geschwindigkeit: `upload_speed = 115200` in platformio.ini
- Reset-Button während Upload-Start drücken

## Performance-Hinweise

### SerialLink Bandbreite
- Baudrate: 9600 bps
- Datenrate: ca. 960 Bytes/s
- Frame-Overhead: ~10 Bytes pro Nachricht
- Bei 5s-Intervall: <1% Bus-Auslastung

### Multi-Tasking mit Kamera
```cpp
// Priorisierte Task-Struktur empfohlen:
void loop() {
    // 1. Kamera (zeitkritisch)
    handleCamera();
    
    // 2. SerialLink (niedrige Priorität)
    if (millis() - lastSerialCheck > 100) {  // Alle 100ms prüfen
        handleSerialLink();
        lastSerialCheck = millis();
    }
    
    // 3. Webserver etc.
    handleWebServer();
}
```

### Speicher-Optimierung
ESP32-CAM hat begrenzt RAM durch Kamera-Buffer:
```cpp
// In SerialLink.h bei Speicherproblemen:
#define SL_MAX_DATA_LENGTH 32  // Statt 64
```

## Anwendungsbeispiele

### 1. Wasserstand auf Webseite anzeigen
```cpp
#include <ESPAsyncWebServer.h>
#include <SerialLink.h>

String currentWaterLevel = "---";

void handleSerialLink() {
    if (serialLink.available()) {
        uint8_t msgType;
        uint8_t msgData[64];
        uint8_t msgLength;
        
        if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
            if (msgType == SL_MSG_WATERLEVEL) {
                WaterLevelData* data = (WaterLevelData*)msgData;
                currentWaterLevel = String(data->level_mm) + " mm";
            }
        }
    }
}

// Im Webserver-Handler:
server.on("/api/waterlevel", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", 
                  "{\"level\":\"" + currentWaterLevel + "\"}");
});
```

### 2. Alarm mit LED-Anzeige
```cpp
#define LED_PIN 4  // Onboard LED oder externe LED

void handleAlarm() {
    if (serialLink.available()) {
        uint8_t msgType;
        uint8_t msgData[64];
        uint8_t msgLength;
        
        if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
            if (msgType == SL_MSG_ALARM_STATE) {
                uint8_t alarmState = msgData[0];
                digitalWrite(LED_PIN, alarmState > 0 ? HIGH : LOW);
            }
        }
    }
}
```

### 3. Daten loggen auf SD-Karte
```cpp
#include <SD_MMC.h>

void logToSD(WaterLevelData* data) {
    File file = SD_MMC.open("/waterlog.csv", FILE_APPEND);
    if (file) {
        file.printf("%lu,%d,%d,%d\n", 
                    data->timestamp, 
                    data->level_mm, 
                    data->adc_value, 
                    data->alarm_state);
        file.close();
    }
}
```

## Pin-Referenz ESP32-CAM

| GPIO | Funktion | SerialLink | Verfügbar |
|------|----------|------------|-----------|
| 0    | XCLK     | -          | ❌ Kamera |
| 1    | U0TXD    | -          | ⚠️ USB    |
| 2    | Flash    | -          | ⚠️ Boot   |
| 3    | U0RXD    | -          | ⚠️ USB    |
| 4    | Flash    | -          | ⚠️ Boot   |
| 5    | VSYNC    | -          | ❌ Kamera |
| 12   | MTDI     | -          | ⚠️ Boot   |
| 13   | MTCK     | **TX**     | ✅ Frei   |
| 14   | MTMS     | **RX**     | ✅ Frei   |
| 15   | MTDO     | -          | ✅ Frei   |
| 16   | PSRAM    | -          | ❌ PSRAM  |
| 18   | Y3       | -          | ❌ Kamera |
| 19   | Y4       | -          | ❌ Kamera |
| 21   | Y5       | -          | ❌ Kamera |
| 22   | PCLK     | -          | ❌ Kamera |
| 23   | HREF     | -          | ❌ Kamera |
| 25   | Y9       | -          | ❌ Kamera |
| 26   | SIOD     | -          | ❌ Kamera |
| 27   | SIOC     | -          | ❌ Kamera |
| 32   | PWDN     | -          | ❌ Kamera |
| 33   | -        | -          | ✅ Frei   |

## Weiterführende Links
- [ESP32-CAM Pinout](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/)
- [SerialLink Hauptdokumentation](SerialLink_README.md)
- [UART ESP32 Dokumentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)

## Lizenz
Siehe LICENSE im Hauptverzeichnis
