/* *******************************************************************
   Webclient
   ***************************************************************** */
#include <Arduino.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>

#include <pumpControl.h>
#include <timeserver.h>
#include <MyLittleFSLib.h>

  #if BOARDTYPE == ESP32
  // for Send-Mail
  #include <ESP_Mail_Client.h>

  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

#else // BOARDTYPE == ESP8266)
    #include <ESP_Mail_Client.h>
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
   #include <ESP8266HTTPClient.h>  // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
#endif

// zertifikat bplaced chain
const char* root_ca_chain = \
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

int errCnt_communication = 0;

void sendPost()
// send data as POST to another webserver
// V3 no Arduino String class for sending data
{


WiFiClientSecure wificlient;
wificlient.setCACert(root_ca_chain);

  HTTPClient client;
  const size_t MESSAGE_SIZE_MAX = 300;                     // maximum bytes for Message Buffer
  char message[MESSAGE_SIZE_MAX];                          // the temporary sending message - html body
  char val[32];                                            // buffer to convert floats and integers before appending

  strcpy(message, "board=");                               // Append chars
  strcat(message, TXT_BOARDID);

  strcat(message, "&levelAct=");
  itoa  (myValueFilteredAct, val, 10);
  strcat(message, val);

  strcat(message, "&debug_level_switches=");
  itoa(debugLevelSwitches, val, 10);
  strcat(message, val);

  strcat(message, "&AHH=");
  itoa(val_AHH, val, 10);
  strcat(message, val);

  strcat(message, "&AH=");
  itoa(val_AH, val, 10);
  strcat(message, val);

  strcat(message, "&AL=");
  itoa(val_AL, val, 10);
  strcat(message, val);

  strcat(message, "&pump1_op=");
  itoa(pump1_op, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_op=");
  itoa(pump2_op, val, 10);
  strcat(message, val);

  strcat(message, "&epochTime=");
  itoa(epochTime, val, 10);
  strcat(message, val);

  strcat(message, "&pump1_operationTime=");
  itoa(pump1_operationTime, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_operationTime=");
  itoa(pump2_operationTime, val, 10);
  strcat(message, val);


  client.begin(wificlient, sendHttpTo);                                        // Specify request destination
  client.addHeader("Content-Type", "application/x-www-form-urlencoded");       // Specify content-type header
  int httpCode = client.POST(message);                                         // Send the request
  client.writeToStream(&Serial);                                               // Debug only: Output of received data
  Serial.print(F("\nhttpCode: "));
  Serial.println(httpCode);  
  Serial.print(F("\nmessage: "));                                                  // Print HTTP return code;
  Serial.println(message);
  client.end();  //Close connection

  if (simError == 1) {
    httpCode = -66;
    simError = 0;   // reset after one error
  }
  if (simReboot == 1) {
    httpCode = -99; // do not reset; 
  }
  // error handling
  if (httpCode<0) {

    // write to file
    String errMessage = "";
    errMessage =  currentDate ;
    errMessage += " - " ;
    errMessage += formattedTime; 
    errMessage += " - " ;
    errMessage +=  " httpCode = ";
    errMessage +=  httpCode ;
    errMessage += "\n";
    appendFile("/error.log", errMessage.c_str());

    errCnt_communication++;
    
    // ein Neustart wg fehlerhafter Kommunikation führt zu instabilem System
    // am wichtigsten ist die Pumpensteuerung
    
    // if (errCnt_communication > ERR_CNT_COMMUNICATION)
    // {
    //   // reset ESP8266
    //   errMessage =  currentDate ;
    //   errMessage += " - " ;
    //   errMessage += formattedTime; 
    //   errMessage += " - " ;
    //   errMessage +=  "client connection error - restart triggered\n";
    //   appendFile("/error.log", errMessage.c_str());
    //   ESP.restart();
    // }
  } else {
    // reset to zero, when communication is running again
    errCnt_communication = 0;
  }
}
