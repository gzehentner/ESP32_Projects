/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  Verdrahtung CAM Photo Development:
  - Umschalter gedrückt -> Versorgungsspannung aus
  - Umschalter nicht gedrückt -> Versorgungsspannung ein
  - Taster gedrückt -> Boot modus

  Verdrahtung CAM Photo Keller
  - Litze Orange Gelb: Versorgungsspannung
  - Litze Braun Violett: Boot moduls

  Firmware Download
  1) Versorgung aus
  2) Taster drücken
  3) Versorgung ein
  4) Download starten
  5) Taster loslassen und Versorgung aus / ein --> Firmware läuft
*********/

#include <U8g2lib.h>
#include <Wire.h>
#include <display_lib.h>
 

#ifdef useElegantOTA
  #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#endif

#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
// #include <StringArray.h>
#include <SPIFFS.h>
#include <SD_MMC.h>
#include <FS.h>
#include <timeserver.h>

// #include <AsyncElegantOTA.h>

#ifdef useElegantOTA
  #include <ElegantOTA.h>
#endif

#include <main.h>

#include <takePhoto.h>
#if 0
#include <SerialLink.h>
#endif

  // camera_fb_t * fb = NULL; // pointer

// SerialLink für Wasserstand-Daten (vorübergehend deaktiviert)
#if 0
SerialLink serialLink;
int lastWaterLevel_mm = 0;
uint8_t lastAlarmState = 0;
unsigned long lastSerialCheck = 0;
unsigned long lastDataReceived = 0;
#endif

// Replace with your network credentials
const char* ssid = "Zehentner";
const char* password = "ElisabethScho";



// variables and constants
long msecLastCapture = 0;
long msecNow         = 0;
long msecLastPrint   = 0;
long msecNowPrint    = 0;
long msecTick        = 0;

// count of communication errors
long errCnt_WiFi = 0;

int genError = 0;
int genErrorDone = 0;

#if isLiveSystem == 1
  #define SEC_CAPTURE_DIFF 60
  const char *myServerName     = "https://zehentner.bplaced.net/Wasserstand/live/rec_photo.php"; 
  const char *myServerNameFile = "https://zehentner.bplaced.net/Wasserstand/live/rec_photo_file.php"; 
#else 
  #define SEC_CAPTURE_DIFF 60
  const char *myServerName     = "https://zehentner.bplaced.net/Wasserstand/dev/rec_photo.php"; 
  const char *myServerNameFile = "https://zehentner.bplaced.net/Wasserstand/dev/rec_photo_file.php"; 
 #endif

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;
bool sdCardReady = false;

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
    <h2>ESP32-CAM Last Photo xy</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
      <button onclick="openPhotoList()">PHOTO LIST</button>
      <button onclick="injectErrorFunc();">INJECT ERROR</button>
      <button onclick="debugFunc();">Show Debug Page</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo" width="70%"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
  }
  function rotatePhoto() {
    var img = document.getElementById("photo");
    deg += 90;
    if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
    else{ document.getElementById("container").className = "hori"; }
    img.style.transform = "rotate(" + deg + "deg)";
  }
  function isOdd(n) { return Math.abs(n % 2) == 1; }
  function injectErrorFunc() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/injectError", true);
    xhr.send();
  }
  function debugFunc() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/debug", true);
    xhr.send();
  }
  function openPhotoList() {
    window.open('/photos', '_blank');
  }

</script>
</html>)rawliteral";




void setup() {
//======================================================================
  // The current hardware supply drops briefly during WiFi startup.
  // Disable brownout resets temporarily so the controller can start.
  // The proper long-term fix is a stable 5 V supply with sufficient current.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  u8g2.begin();
  u8g2_prepare();
  msecTick = 0;

  //======================================================================
  // Serial port for debugging purposes
  Serial.begin(115200);

  // MAC-Adresse ausgeben
  Serial.print("MAC-Address: "); 
  Serial.println(WiFi.macAddress());

  //======================================================================
  // prepare WiFi
  Serial.println("WiFi init start");
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("CamDev");

  // Connect to Wi-Fi with timeout to avoid blocking setup forever.
  WiFi.begin(ssid, password);
  unsigned long wifiStart = millis();
  const unsigned long wifiTimeoutMs = 30000;
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart) < wifiTimeoutMs) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
  } else {
    Serial.println("WiFi connect timeout - starting fallback AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-CAM-Photo");
    Serial.print("AP IP Address: http://");
    Serial.println(WiFi.softAPIP());
  }

  //======================================================================
  // prepare file system 
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed (auto-format path). Trying explicit format...");

    if (!SPIFFS.format()) {
      Serial.println("SPIFFS format failed");
      ESP.restart();
    }

    if (!SPIFFS.begin(false)) {
      Serial.println("SPIFFS mount failed after explicit format");
      ESP.restart();
    }

    Serial.println("SPIFFS formatted and mounted successfully");
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
  // Temporary test mode: format SPIFFS on every restart.
  Serial.println("Formatting SPIFFS...");
  SPIFFS.end();

  if (!SPIFFS.format()) {
    Serial.println("SPIFFS format failed");
    ESP.restart();
  }

  if (!SPIFFS.begin(false)) {
    Serial.println("SPIFFS mount failed after format");
    ESP.restart();
  }

  Serial.println("SPIFFS formatted and mounted successfully");

  //======================================================================
  // SPIFFS-only mode: SD initialization is disabled to avoid pin conflicts
  // with I2C on GPIO14/15 (used by the display setup).
  sdCardReady = false;
  Serial.println("SD init skipped (SPIFFS-only mode)");

  //======================================================================
  // print error file
  // Serial.println("Error file:");
  // File file = SPIFFS.open("/error.log", FILE_READ); 
  // if (!file) { 
  //   Serial.println("Datei öffnen fehlgeschlagen");
  //    return; 
  // }

  // // Dateiinhalt lesen und im Terminal ausgeben
  // while (file.available()) {
  //   Serial.write(file.read());
  // }

  // // Datei schließen
  // file.close();

  /*=================================================================*/
  /*====================   Prepare connection to timeserver */

  #if true
    // ESP32 
    configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
    setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
    tzset();
  #else
    // ESP8266
    configTime(MY_TZ, MY_NTP_SERVER);    // --> for the ESP8266 only
  #endif

  //======================================================================
  // Print ESP32 Local IP Address
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP Address: http://");
    Serial.println(WiFi.localIP());
  } else if (WiFi.getMode() & WIFI_AP) {
    Serial.print("Use AP IP Address: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("No IP address (WiFi not connected)");
  }

  //======================================================================
  // OV2640 camera module
  // Zero-initialize all fields to avoid random values in camera options
  // which were added in newer versions of the camera driver.
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM;

  // if (psramFound()) {
  if (false) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    // for test use SVGA for ever
    config.frame_size = FRAMESIZE_SVGA;
    //config.frame_size = FRAMESIZE_96X96;

    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  //======================================================================
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    // prepare display
    u8g2.clearBuffer();
    u8g2_prepare();

    // prepare first line
    u8g2.setCursor(0, 0);
    u8g2.print("Camera init failed with error 0x%x");
    u8g2.print(err);
    
    ESP.restart();
  }

  #ifdef useElagantOTA
     // Initialisiere den ElegantOTA Server 
     ElegantOTA.begin(&server); 
  #endif

  //======================================================================
  // SerialLink vorübergehend deaktiviert

  //======================================================================

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    takeNewPhoto = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/injectError", HTTP_GET, [](AsyncWebServerRequest * request) {
    genError = 1;
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  // Show list of timestamped photos stored in SPIFFS.
  server.on("/photos", HTTP_GET, [](AsyncWebServerRequest *request) {
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory()) {
      request->send(500, "text/plain", "Fehler beim Lesen von SPIFFS");
      return;
    }

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>SPIFFS Photos</title></head><body>";
    html += "<h2>SPIFFS Photo List</h2>";
    html += "<p><a href='/'>Back to Camera Page</a></p><ul>";

    File file = root.openNextFile();
    int count = 0;
    while (file) {
      String rawName = String(file.name());
      String normalizedName = rawName;
      if (!normalizedName.startsWith("/")) {
        normalizedName = "/" + normalizedName;
      }

      if (normalizedName.startsWith("/photo_") && normalizedName.endsWith(".jpg")) {
        html += "<li><a href='/photo?name=" + normalizedName + "' target='_blank'>";
        html += normalizedName + "</a> (" + String((unsigned long)file.size()) + " bytes)</li>";
        count++;
      }
      file.close();
      file = root.openNextFile();
    }

    root.close();

    if (count == 0) {
      html += "<li>No timestamp photos found in SPIFFS yet.</li>";
    }

    html += "</ul></body></html>";
    request->send(200, "text/html", html);
  });

  // Serve a selected photo from SPIFFS by filename parameter.
  server.on("/photo", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("name")) {
      request->send(400, "text/plain", "Missing parameter: name");
      return;
    }

    String fileName = request->getParam("name")->value();
    if (!fileName.startsWith("/")) {
      fileName = "/" + fileName;
    }

    if (!fileName.startsWith("/photo_") || !fileName.endsWith(".jpg")) {
      request->send(400, "text/plain", "Invalid filename");
      return;
    }

    if (!SPIFFS.exists(fileName)) {
      request->send(404, "text/plain", "File not found");
      return;
    }

    request->send(SPIFFS, fileName, "image/jpg", false);
  });

  // handle DebugPage
  //======================================================================
  server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    File file = SPIFFS.open("/error.log", "r");
    if (!file) {
      request->send_P(500, "text/plain", "Fehler beim Öffnen der Datei");
      return;
    }
    String data = file.readString();
    file.close();

    String html = "<!DOCTYPE html><html><head><title>Datenanzeige</title></head><body>";
    html += "<h1>Daten aus SPIFFS</h1>";
    html += "<pre>" + data + "</pre>";
    html += "</body></html>";

    request->send(200, "text/html", html);

  });
    
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", "Hi! This is ElegantOTA Demo. From Schorsch");
      });
 

  // Start server
  server.begin();

  msecLastCapture = millis(); 
  msecLastPrint = millis();
  // count seconds

}

void loop() {

 
  #ifdef useElegantOTA
    ElegantOTA.loop();
  #endif
  
  //======================================================================
  // SerialLink vorübergehend deaktiviert
  
  //======================================================================
  // take photo on click
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    // postImageOnly();  // Temporarily disabled for stability test
    takeNewPhoto = false;
  }

  //======================================================================
  // take photo every n miliseconds and save to SD Card
  msecNow = millis();
  if ((msecNow - msecLastCapture)/1000 > SEC_CAPTURE_DIFF)
  // if (false)
  {
    Serial.println("capturing photo");
    capturePhotoSaveSpiffs();
    // postImageOnly();  // Temporarily disabled for stability test

    // Check if SD Card is available and save to SD, otherwise just keep in SPIFFS
    if (sdCardReady) {
      capturePhotoSaveSD();
      Serial.println("Photo saved to SD Card");
    } else {
      Serial.println("SD Card not available - photo saved to SPIFFS only");
    }

    msecLastCapture = msecNow;
    Serial.println("Photo capture done");
    // reset msecTick with every capture
    msecTick = 0;
  }

  //======================================================================
  // print strint on display every second

  if ((msecNow - msecLastPrint)/1000 > 1)
  {
    
    msecLastPrint = msecNow;
    
    // prepare display
    u8g2.clearBuffer();
    u8g2_prepare();

    // prepare first line
    u8g2.setCursor(0, 0);
    u8g2.print("last: ");
    u8g2.print(msecTick);
    
    // second line below
  
    if (WiFi.status() != WL_CONNECTED)  {
      u8g2.setCursor(0, 12);
      u8g2.print("WiFi status: ");
      u8g2.print(WiFi.status());
    }
    else {
      u8g2.setCursor(0, 12);
      u8g2.print("WiFi connected");
    }

    u8g2.setCursor(0, 24);
    u8g2.print("ErrCnt:");
    u8g2.print(errCnt_WiFi);

    // SD-Status anzeigen (4. Zeile)
    u8g2.setCursor(0, 36);
    if (sdCardReady) {
      u8g2.print("SD: OK");
    } else {
      u8g2.print("SD: FAIL");
    }

    u8g2.sendBuffer();
    msecTick++;

  }


}
