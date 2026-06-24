# SD-Karte Konfiguration für ESP32-CAM AI Thinker

## Funktion
Das Projekt wurde umgebaut, um Fotos automatisch **alle 10 Minuten** auf der **integrierten SD-Karte** des ESP32-CAM zu speichern statt auf einem externen Server hochzuladen.

Dateinamen folgen dem Format: `photo_YYYY_MM_DD_HH_MM_SS.jpg` (z.B. `photo_2026_03_18_14_30_45.jpg`)

## SD-Karte - Integrierter Slot

Das ESP32-CAM AI Thinker hat einen **integrierten microSD-Kartenslot** mit vordefinierten Pins.

### Pin-Belegung (ESP32-CAM AI Thinker)

| Funktion | GPIO Pin | Beschreibung |
|----------|----------|--------------|
| CS       | GPIO 4   | Chip Select  |
| MOSI     | GPIO 15  | Data Out     |
| MISO     | GPIO 2   | Data In      |
| CLK      | GPIO 14  | Clock        |

### Physikalischer Slot

Der microSD-Kartenslot befindet sich auf der **Rückseite des ESP32-CAM Moduls**.
- Typ: **microSD (nicht full-size SD!)**
- Max. Größe: 2TB SDXC
- Format: **FAT32** empfohlen

### Verkabelung

❌ **KEINE Verkabelung nötig!** Die SD-Karte wird direkt in den integrierten Slot gesteckt.

**Einfach microSD-Karte in den Slot auf der Rückseite einschieben!**

## Funktionsweise

### Beim Start:
- ESP32 versucht SD-Karte zu lesen
- Im Serial Monitor werden Informationen über Kartentyp und Größe angezeigt
- Falls SD-Karte nicht vorhanden/lesbar: Funktioniert im SPIFFS-only Modus

### Zyklisch (alle 10 Minuten):
1. Foto wird gemacht
2. Foto wird in SPIFFS gespeichert (als /photo.jpg - für Web-Interface)
3. Falls SD-Karte verfügbar: Foto wird mit Timestamp-Dateinamen auf SD-Karte gespeichert
4. Display und Serial Monitor zeigen Status an

### Web-Interface:
- `/capture` - Erzeugt manuell ein Foto
- `/` - Zeigt das zuletzt gespeicherte Foto an (aus SPIFFS)

## Fehlerbehandlung

### "SD Card Mount Failed"
- microSD-Karte nicht eingesteckt
- Karte nicht erkannt (evtl. defekt)
- Format-Problem (sollte FAT32 sein)
- Karte mit Adapter versuchen neu einzustecken

### "No SD Card detected"
- Kartekontakt könnte verschmutzt sein
- Mit leichtem Druck neu einstecken
- Mit anderem Kartenleser am PC prüfen

### "Photo saved to SPIFFS only"
- SD-Karte nicht bereit oder defekt
- Projekt funktioniert trotzdem mit SPIFFS (Fotos auf dem Modul gespeichert)

## Vorbereitung der SD-Karte

1. **microSD-Karte formatieren (am PC)**:
   - Karteleser verwenden
   - Mit FAT32 formatieren
   - Größe: Mindestens 256MB, empfohlen: 4GB oder mehr

2. **Karte einstecken**:
   - SD-Karte mit **Gold-Kontakten nach innen** einführen
   - Bis er "klickt" - Karte schnellt dann zurück
   - Ein erneutes Klicken zum Entfernen

3. **ESP32-CAM booten**:
   - Code hochladen
   - Serial Monitor öffnen
   - Erkennungsmeldung beobachten

## Dateigröße

Typische JPEG-Größen (SVGA, Quality=12):
- ~100-150 KB pro Foto
- Bei 10-Minuten-Intervall: ~8-12 MB pro Stunde, ~200-300 MB pro Tag
- Eine 8GB microSD-Karte: ~25-30 Tage Speicherkapazität

## Automatische Dateiorganisation

Fotos werden im Root der SD-Karte mit folgender Nomenklatur gespeichert:
```
/photo_2026_03_18_14_30_45.jpg
/photo_2026_03_18_14_40_52.jpg
/photo_2026_03_18_14_51_03.jpg
...
```

Um Ordnung zu halten, sollten die Dateien regelmäßig (z.B. monatlich) manuell oder per Skript in Ordner verschoben werden.

## Monitor Output Beispiel

```
Connecting to WiFi...
WiFi connected
IP Address: http://192.168.x.x
SPIFFS mounted successfully
SD Card mounted successfully
SD Card Type: SDHC
SD Card Size: 7960MB
ElegantOTA bereit
Initialisiere SerialLink...
SerialLink bereit (RX:14, TX:13)
...
capturing photo
Picture file name: /photo_2026_03_18_14_30_45.jpg
The picture has been saved: /photo_2026_03_18_14_30_45.jpg - Size: 123456 bytes
Photo saved to SD Card
Photo capture done
```
