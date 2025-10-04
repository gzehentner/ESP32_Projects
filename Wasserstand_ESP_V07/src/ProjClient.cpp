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
#include <UrlEncode.h>

#else // BOARDTYPE == ESP8266)
#include <ESP_Mail_Client.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h> // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
#include <UrlEncode.h>
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
// 3. error.log Datei löschen und neu erstellen
//     Fehlermeldung fd=49 "reset by peer" deutet darauf hin, dass die datei übergelaufen ist

const char *root_ca_chain =
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

int errCnt_communication = 0;

void sendPost(PumpStatus &pumpStatus, PumpControl &pumpControl)
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
  itoa(pumpControl.pump1_op, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_op=");
  itoa(pumpControl.pump2_op, val, 10);
  strcat(message, val);

  strcat(message, "&epochTime=");
  itoa(epochTime, val, 10);
  strcat(message, val);

  strcat(message, "&pump1_operationTime=");
  itoa(pumpStatus.pump1_operationTime, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_operationTime=");
  itoa(pumpStatus.pump2_operationTime, val, 10);
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

void sendWhatsAppMessage(String message)
{

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  //String url = "https://api.callmebot.com/whatsapp.php?phone=+491607547424&apikey=6878208&text=" + urlEncode(message);
  

  HTTPClient client;
  client.begin(url);

  // Specify content-type header
  client.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = client.POST(url);
  if (httpResponseCode == 200)
  {
    Serial.print("Message sent successfully");
  }
  else
  {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  client.end();
}
