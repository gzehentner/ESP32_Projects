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
// !!! neu
// ich habe jetzt ein Zertifikat von Lets encrypt über folgenden Link:
//   https://letsencrypt.org/certs/isrgrootx1.pem
// das soll 10 Jahre halten

#ifdef blaced_root_cert

const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFBTCCAu2gAwIBAgIQWgDyEtjUtIDzkkFX6imDBTANBgkqhkiG9w0BAQsFADBP\n"
    "MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy\n"
    "Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa\n"
    "Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF\n"
    "bmNyeXB0MQwwCgYDVQQDEwNSMTMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
    "AoIBAQClZ3CN0FaBZBUXYc25BtStGZCMJlA3mBZjklTb2cyEBZPs0+wIG6BgUUNI\n"
    "fSvHSJaetC3ancgnO1ehn6vw1g7UDjDKb5ux0daknTI+WE41b0VYaHEX/D7YXYKg\n"
    "L7JRbLAaXbhZzjVlyIuhrxA3/+OcXcJJFzT/jCuLjfC8cSyTDB0FxLrHzarJXnzR\n"
    "yQH3nAP2/Apd9Np75tt2QnDr9E0i2gB3b9bJXxf92nUupVcM9upctuBzpWjPoXTi\n"
    "dYJ+EJ/B9aLrAek4sQpEzNPCifVJNYIKNLMc6YjCR06CDgo28EdPivEpBHXazeGa\n"
    "XP9enZiVuppD0EqiFwUBBDDTMrOPAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG\n"
    "MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/\n"
    "AgEAMB0GA1UdDgQWBBTnq58PLDOgU9NeT3jIsoQOO9aSMzAfBgNVHSMEGDAWgBR5\n"
    "tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG\n"
    "Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD\n"
    "VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B\n"
    "AQsFAAOCAgEAUTdYUqEimzW7TbrOypLqCfL7VOwYf/Q79OH5cHLCZeggfQhDconl\n"
    "k7Kgh8b0vi+/XuWu7CN8n/UPeg1vo3G+taXirrytthQinAHGwc/UdbOygJa9zuBc\n"
    "VyqoH3CXTXDInT+8a+c3aEVMJ2St+pSn4ed+WkDp8ijsijvEyFwE47hulW0Ltzjg\n"
    "9fOV5Pmrg/zxWbRuL+k0DBDHEJennCsAen7c35Pmx7jpmJ/HtgRhcnz0yjSBvyIw\n"
    "6L1QIupkCv2SBODT/xDD3gfQQyKv6roV4G2EhfEyAsWpmojxjCUCGiyg97FvDtm/\n"
    "NK2LSc9lybKxB73I2+P2G3CaWpvvpAiHCVu30jW8GCxKdfhsXtnIy2imskQqVZ2m\n"
    "0Pmxobb28Tucr7xBK7CtwvPrb79os7u2XP3O5f9b/H66GNyRrglRXlrYjI1oGYL/\n"
    "f4I1n/Sgusda6WvA6C190kxjU15Y12mHU4+BxyR9cx2hhGS9fAjMZKJss28qxvz6\n"
    "Axu4CaDmRNZpK/pQrXF17yXCXkmEWgvSOEZy6Z9pcbLIVEGckV/iVeq0AOo2pkg9\n"
    "p4QRIy0tK2diRENLSF2KysFwbY6B26BFeFs3v1sYVRhFW9nLkOrQVporCS0KyZmf\n"
    "wVD89qSTlnctLcZnIavjKsKUu1nA1iU0yYMdYepKR7lWbnwhdx3ewok=\n"
    "-----END CERTIFICATE-----\n";

#else
// using the root certification from Lets encrypt - should be valid since 2035
const char *root_ca =

    // const char *root_ca_chain =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";
#endif

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
