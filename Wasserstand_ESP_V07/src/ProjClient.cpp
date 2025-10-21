/* *******************************************************************
   Webclient
   ***************************************************************** */
#include <Arduino.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>

#include <pumpControl.h>
#include <timeserver.h>
#include <MyLittleFSLib.h>
// #include <TelnetStream.h>
// #include <debugPrint.h>

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
// !!! neu
// ich habe jetzt ein Zertifikat von Lets encrypt über folgenden Link:
//   https://letsencrypt.org/certs/isrgrootx1.pem
// das soll 10 Jahre halten

#define bplaced_root_cert
#ifdef  bplaced_root_cert
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

#else
// usint the root certification from Lets encrypt - should be valid since 2035
const char *root_ca_chain =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";
#endif

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
