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

// zertifikat bplaced chain
// Wie erzeuge ich das Zertifikat?
// 1. OpenSSL starten
//    - in das Verzeichnis C:\Program Files\OpenSSL-Win64 wechseln
//    - start.bat doppelt klicken
// 2. In der OpenSSL Konsole:
//    - openssl s_client -showcerts -connect zehentner.bplaced.net:443  
//    - in der Ausgabe sind zwei Zertifikate enthalten:
//      - erkennbar an "BEGIN CERTIFICATE" und "END CERTIFICATE"
//      - Den Bereich zwischen diesen beiden Zeilen einschließlich der Zeilen selbst kopieren
//      - das zweite Zertifikat ist das Root-Zertifikat, dieses verwenden wir
//      - im notepad++ formatieren (siehe unten)

// Root CA certificate of the server
//const char* root_ca_chain = \

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\n" \
"WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n" \
"CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ\n" \
"DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG\n" \
"AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy\n" \
"6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw\n" \
"SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP\n" \
"Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB\n" \
"hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB\n" \
"/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU\n" \
"ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC\n" \
"hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG\n" \
"A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN\n" \
"AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y\n" \
"v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38\n" \
"01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1\n" \
"e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn\n" \
"UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV\n" \
"aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z\n" \
"WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R\n" \
"PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q\n" \
"pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo\n" \
"6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV\n" \
"uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA\n" \
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
