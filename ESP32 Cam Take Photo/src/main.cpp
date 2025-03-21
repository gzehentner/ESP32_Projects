/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
  
  IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
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
//#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>
#include <timeserver.h>

// #include <AsyncElegantOTA.h>

#ifdef useElegantOTA
  #include <ElegantOTA.h>
#endif

#include <main.h>

#include <takePhoto.h>    

  // camera_fb_t * fb = NULL; // pointer

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
  #define SEC_CAPTURE_DIFF 20
  const char *myServerName     = "https://zehentner.bplaced.net/Wasserstand/dev/rec_photo.php"; 
  const char *myServerNameFile = "https://zehentner.bplaced.net/Wasserstand/dev/rec_photo_file.php"; 
 #endif

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;

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

</script>
</html>)rawliteral";




void setup() {
//======================================================================
  // Turn-off the 'brownout detector'
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
  char myhostname[8] = {"CamDev"};
  WiFi.hostname(myhostname);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  //======================================================================
  // prepare file system 
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

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
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  //======================================================================
  // OV2640 camera module
  camera_config_t config;
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
  // take photo on click
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
  }

  //======================================================================
  // take photo every n miliseconds and post it to external server
  msecNow = millis();
  if ((msecNow - msecLastCapture)/1000 > SEC_CAPTURE_DIFF)
  // if (false)
  {
    Serial.println("capturing photo");
    capturePhotoSaveSpiffs();

    Serial.println("Sending photo started");
    postImageOnly();

    msecLastCapture = msecNow;
    Serial.println("Sending photo done");

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

    u8g2.sendBuffer();
    msecTick++;

  }


}