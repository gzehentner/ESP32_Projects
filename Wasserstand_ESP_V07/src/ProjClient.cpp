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
#include <ESP8266HTTPClient.h> // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
#endif

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

const char *root_ca_chain =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw\n"
    "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
    "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\n"
    "WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
    "RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n"
    "CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ\n"
    "DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG\n"
    "AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy\n"
    "6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw\n"
    "SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP\n"
    "Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB\n"
    "hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB\n"
    "/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU\n"
    "ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC\n"
    "hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG\n"
    "A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN\n"
    "AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y\n"
    "v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38\n"
    "01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1\n"
    "e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn\n"
    "UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV\n"
    "aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z\n"
    "WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R\n"
    "PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q\n"
    "pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo\n"
    "6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV\n"
    "uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA\n"
    "-----END CERTIFICATE-----\n";

int errCnt_communication = 0;

void sendPost(PumpStatus &pumpControl)
// send data as POST to another webserver
// V3 no Arduino String class for sending data
{

  WiFiClientSecure wificlient;
  wificlient.setCACert(root_ca_chain);

  HTTPClient client;
  const size_t MESSAGE_SIZE_MAX = 300; // maximum bytes for Message Buffer
  char message[MESSAGE_SIZE_MAX];      // the temporary sending message - html body
  char val[32];                        // buffer to convert floats and integers before appending

  strcpy(message, "board="); // Append chars
  strcat(message, TXT_BOARDID);

  strcat(message, "&levelAct=");
  itoa(myValueFilteredAct, val, 10);
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
  itoa(pumpControl.pump1_operationTime, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_operationTime=");
  itoa(pumpControl.pump2_operationTime, val, 10);
  strcat(message, val);

  client.begin(wificlient, sendHttpTo);                                  // Specify request destination
  client.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
  int httpCode = client.POST(message);                                   // Send the request
  client.writeToStream(&Serial);                                         // Debug only: Output of received data
  Serial.print(F("\nhttpCode: "));
  Serial.println(httpCode);
  Serial.print(F("\nmessage: ")); // Print HTTP return code;
  Serial.println(message);
  client.end(); // Close connection

  if (simError == 1)
  {
    httpCode = -66;
    simError = 0; // reset after one error
  }
  if (simReboot == 1)
  {
    httpCode = -99; // do not reset;
  }
  // error handling
  if (httpCode < 0)
  {

    // write to file
    String errMessage = "";
    errMessage = currentDate;
    errMessage += " - ";
    errMessage += formattedTime;
    errMessage += " - ";
    errMessage += " httpCode = ";
    errMessage += httpCode;
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
  }
  else
  {
    // reset to zero, when communication is running again
    errCnt_communication = 0;
  }
}
