// ===================================================
//
// Utilities to capture photo
//
// ===================================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_camera.h>
#include <timeserver.h>

#include <SPIFFS.h>

#include <main.h>
#include <takePhoto.h>

int errCnt_communication = 0;

// Root CA certificate of the server
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBTCCAu2gAwIBAgIQS6hSk/eaL6JzBkuoBI110DANBgkqhkiG9w0BAQsFADBP\n" \
"MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy\n" \
"Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa\n" \
"Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF\n" \
"bmNyeXB0MQwwCgYDVQQDEwNSMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQDPV+XmxFQS7bRH/sknWHZGUCiMHT6I3wWd1bUYKb3dtVq/+vbOo76vACFL\n" \
"YlpaPAEvxVgD9on/jhFD68G14BQHlo9vH9fnuoE5CXVlt8KvGFs3Jijno/QHK20a\n" \
"/6tYvJWuQP/py1fEtVt/eA0YYbwX51TGu0mRzW4Y0YCF7qZlNrx06rxQTOr8IfM4\n" \
"FpOUurDTazgGzRYSespSdcitdrLCnF2YRVxvYXvGLe48E1KGAdlX5jgc3421H5KR\n" \
"mudKHMxFqHJV8LDmowfs/acbZp4/SItxhHFYyTr6717yW0QrPHTnj7JHwQdqzZq3\n" \
"DZb3EoEmUVQK7GH29/Xi8orIlQ2NAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG\n" \
"MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/\n" \
"AgEAMB0GA1UdDgQWBBS7vMNHpeS8qcbDpHIMEI2iNeHI6DAfBgNVHSMEGDAWgBR5\n" \
"tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG\n" \
"Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD\n" \
"VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B\n" \
"AQsFAAOCAgEAkrHnQTfreZ2B5s3iJeE6IOmQRJWjgVzPw139vaBw1bGWKCIL0vIo\n" \
"zwzn1OZDjCQiHcFCktEJr59L9MhwTyAWsVrdAfYf+B9haxQnsHKNY67u4s5Lzzfd\n" \
"u6PUzeetUK29v+PsPmI2cJkxp+iN3epi4hKu9ZzUPSwMqtCceb7qPVxEbpYxY1p9\n" \
"1n5PJKBLBX9eb9LU6l8zSxPWV7bK3lG4XaMJgnT9x3ies7msFtpKK5bDtotij/l0\n" \
"GaKeA97pb5uwD9KgWvaFXMIEt8jVTjLEvwRdvCn294GPDF08U8lAkIv7tghluaQh\n" \
"1QnlE4SEN4LOECj8dsIGJXpGUk3aU3KkJz9icKy+aUgA+2cP21uh6NcDIS3XyfaZ\n" \
"QjmDQ993ChII8SXWupQZVBiIpcWO4RqZk3lr7Bz5MUCwzDIA359e57SSq5CCkY0N\n" \
"4B6Vulk7LktfwrdGNVI5BsC9qqxSwSKgRJeZ9wygIaehbHFHFhcBaMDKpiZlBHyz\n" \
"rsnnlFXCb5s8HKn5LsUgGvB24L7sGNZP2CX7dhHov+YhD+jozLW2p9W4959Bz2Ei\n" \
"RmqDtmiXLnzqTpXbI+suyCsohKRg6Un0RC47+cpiVwHiXZAW+cn8eiNIjqbVgXLx\n" \
"KPpdzvvtTnOPlC7SQZSYmdunr3Bf9b77AiC/ZidstK36dRILKz7OA54=\n" \
"-----END CERTIFICATE-----\n";


// =======================================================================================
// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
// =======================================================================================
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// =======================================================================================
// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
// =======================================================================================
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // ====================================
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // ====================================
    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // ====================================
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
    // ====================================
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // ====================================
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // ====================================
    // check if file has been correctly saved in SPIFFS
    // ====================================
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}


// ====================================
// Capture Photo and Send to bplaced server
void capturePhotoPost( void ) {
// ====================================
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly
  unsigned long timeCaptureMs = 0;
  unsigned long timeNow = 0;

  do {
    // ====================================================================
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
   // Überprüfen, ob das Foto vollständig aufgenommen wurde 
   if (fb->len == 0) { 
    Serial.println("Foto ist unvollständig oder leer"); 
    esp_camera_fb_return(fb); 
    return;
   }
    // ====================================================================
    // Send photo to server
    // ====================================
    if (WiFi.status() == WL_CONNECTED) {


      /*=================================================================*/
      /*  code for getting time from NTP       */
      getEpochTime(epochTime);
      formatDateAndTime(formattedTime, currentDate,epochTime);
      /* End getting time and date */
    
      // ====================================
        HTTPClient http;
        
        // HTTP-POST-Anfrage vorbereiten 
        http.begin(myServerName, root_ca);  // Use HTTPS with the root CA certificate
        http.addHeader("Content-Type", "multipart/form-data; boundary=boundary");

        // ====================================
        // Bild als Formulardaten hinzufügen 
        String body = "--boundary\r\n"; 
        body += "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n"; 
        body += "Content-Type: image/jpeg\r\n\r\n"; 
        body += String((char*)fb->buf, fb->len); 
        body += "\r\n--boundary\r\n"; 
        // body += "Content-Disposition: form-data; name=\"text\"\r\n\r\n"; 
        // body += "Hier ist der Text, den du senden möchtest.\r\n"; 
        // body += "--boundary--\r\n"; 

        // ====================================
        // debug prints
        // ====================================
        // Serial.print("Length: "); Serial.println(fb->len);
        // Serial.print("time: "); Serial.println(epochTime);

        // ====================================
        // POST-Anfrage senden 
        int httpResponseCode = http.POST((uint8_t*)body.c_str(), body.length()); 
        
        if(httpResponseCode > 0) { 
          String response = http.getString(); 
          Serial.println(httpResponseCode); 
          Serial.println(response); 
        } else { 
          Serial.print("Error on sending POST: "); 
          Serial.println(httpResponseCode); 
        }

        http.end();


      // ====================================================================
      // Debug error.log
      // ====================================================================
      // if (genError==1){
      //   httpResponseCode = -1000;
      //   genError = 0;
      // }


      // ====================================================================
      // error handling
      // ====================================================================
      // if (httpResponseCode<0) {

      //   File file = SPIFFS.open("/error.log", FILE_APPEND);
      //   if (!file) { 
      //     Serial.println("Datei öffnen fehlgeschlagen"); 
      //     return; 
      //   }

      //   // ==================================
      //   // write error message to file
      //   String errMessage = "";
      //   errMessage =  currentDate + " - " + formattedTime + " - " +  " httpCode = " +  httpResponseCode + "\n";

      //   file.println(errMessage.c_str());

      //   errCnt_communication++;

      //   Serial.println("Error occured");
      //   Serial.print("Error-Count: ");
      //   Serial.println(errCnt_communication);

      //   if (errCnt_communication > ERR_CNT_COMMUNICATION)
      //   {
      //     // ==================================
      //     // reset ESP8266
      //     // ==================================
      //     errMessage = currentDate + " - " + formattedTime + " - " +  "client connection error - restart triggered\n";
      //     file.println(errMessage.c_str());

      //     Serial.println("Too much errors. Restrt triggered");
      //     file.close();
      //     delay(1000);
          
      //     ESP.restart();
      //   }
      //   file.close();

      // } else {
      //   Serial.println("ErrorCount reset");
      //   Serial.print("Error-Count: ");
      //   Serial.println(errCnt_communication);        
        
      //   errCnt_communication = 0;
      // }
    }

    // check photo
    if (fb->len>100) {
      ok = true;

    } else {
      ok = false;
    }
    // Return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
  } while ( !ok );
}

// ========================================================================================================
void postImageFile() {
// ========================================================================================================

  // Öffne die JPEG-Datei aus dem SPIFFS
  File file = SPIFFS.open("/image.jpg", "r");
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei");
    return;
  }

  // HTTP-POST-Anfrage vorbereiten
  HTTPClient http;
  http.begin(myServerName, root_ca);  // Use HTTPS with the root CA certificate
  http.addHeader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

  // Bild als Formulardaten hinzufügen
  String body = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
  body += "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n";
  body += "Content-Type: image/jpeg\r\n\r\n";

  // File meta data
  Serial.print(" - Size: ");
  Serial.print(file.size());
  Serial.println(" bytes");

  // Bilddaten lesen und zum Body hinzufügen
  while (file.available()) {
    body += (char)file.read();
  }

  file.close();
  body += "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

  int httpResponseCode = http.POST((uint8_t*)body.c_str(), body.length());

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.println("Fehler bei der HTTP-Anfrage");
  }

  http.end();
}

// ========================================================================================================
void postImageOnly() {
// ========================================================================================================

  // Öffne die JPEG-Datei aus dem SPIFFS
  File file = SPIFFS.open(FILE_PHOTO, "r");
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei");
    return;
  }


// Ausgabe der Dateigröße 
  size_t fileSize = file.size(); 
  Serial.print("Dateigröße: "); 
  Serial.print(fileSize); 
  Serial.println(" Bytes");

  uint8_t *buffer = new uint8_t[fileSize];
  file.read(buffer, fileSize);
  file.close();

  //Serial.print("size: "); Serial.println(file.size);

  // HTTP-POST-Anfrage vorbereiten
  HTTPClient http;

  http.begin(myServerName, root_ca);  // Use HTTPS with the root CA certificate
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(buffer, fileSize);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.println("Fehler bei der HTTP-Anfrage");
  }

  http.end();
  delete[] buffer;
}
