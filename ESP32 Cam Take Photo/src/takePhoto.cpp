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
        http.begin(myServerName); 
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
  http.begin(myServerName);
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

  http.begin(myServerName);
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


// ========================================================================================================
