/* *******************************************************************
   Webclient
   ***************************************************************** */
#include <Arduino.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>

  #if BOARDTYPE == ESP32
  // for Send-Mail
  #include <ESP_Mail_Client.h>

  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <HTTPClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

#else // BOARDTYPE == ESP8266)
   #include <ESP_Mail_Client.h>
   #include <ESP8266HTTPClient.h>  // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
#endif

void sendPost()
// send data as POST to another webserver
// V3 no Arduino String class for sending data
{

WiFiClient wificlient;
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


client.begin(wificlient, sendHttpTo);                                        // Specify request destination
client.addHeader("Content-Type", "application/x-www-form-urlencoded");       // Specify content-type header
int httpCode = client.POST(message);                                         // Send the request
client.writeToStream(&Serial);                                               // Debug only: Output of received data
Serial.print(F("\nhttpCode: "));                                             
Serial.println(httpCode);  
Serial.print(F("\nmessage: "));                                                  // Print HTTP return code;
Serial.println(message);
client.end();  //Close connection
}


// *************************************************************
// !!! subscription cancelled --> access to AskSensors not longer possible
//
// AskSensors IoT Platform
// *************************************************************
// settings for AskSensors
const char* apiKeyIn = "i0XfMNeUxnAYSZGoVarg0TGKfXORZjwk";      // API KEY IN
const unsigned int writeInterval = 25000;   // write interval (in ms)

// ASKSENSORS API host name
String host = "api.asksensors.com";
const int https_port = 443;                        // https port
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     // ASKSENSORS HTTPS SHA1 certificate

// *************************************************************
void sendPostToAskSensors() {
// *************************************************************

// create ASKSENSORS client
WiFiClientSecure client;
WiFiClient wificlient;

client.setInsecure();
String https_host = host;

// Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }
  // Create a URL for the request
  String url = "/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += myValueFilteredAct;
  url += "&module2=";
  url += val_AHH;
  url += "&module3=";
  url += val_AH;
  url += "&module4=";
  url += val_AL;
  
  Serial.print("********** requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + https_host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("> Request sent to ASKSENSORS");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
        String line = client.readStringUntil('\n');
        Serial.println("********** ASKSENSORS replay:");
        Serial.println(line);
        Serial.println("********** closing connection");
      
        break;
    }

  }

}
// void sendPost_V2()
// // send data as POST to another webserver
// // V2 no String class for sending data 
// // uses String class for receiving data
// {
//   WiFiClient wificlient;
//   HTTPClient client;
//   const uint16_t MESSAGE_SIZE_MAX = 300;                   // maximum bytes for Message Buffer
//   char message[MESSAGE_SIZE_MAX];                          // the temporary sending message - html body
//   char val[32];                                            // to convert floats and integers before appending

//   strcpy(message, "board=");
//   strcat(message, TXT_BOARDID);

//   strcat(message, "&vcc=");
//   itoa(ESP.getVcc(), val, 10);
//   strcat(message, val);

//   strcat(message, "&output1=1");
//   strcat(message, val);

//   strcat(message, "&output2=2");
//   strcat(message, val);

//   strcat(message, "&button1=3");
//   strcat(message, val);

//   float example = 1234.5678;                               // example how to bring floats into the message
//   strcat(message, "&float=");
//   dtostrf(example, 6, 2, val);
//   strcat(message, val);

//   client.begin(wificlient, sendHttpTo);                                        // Specify request destination
//   client.addHeader("Content-Type", "application/x-www-form-urlencoded");       // Specify content-type header

//   int httpCode = client.POST(message);    // Send the request
//   String payload = client.getString();    // Get the response payload

//   Serial.println(F("D38"));
//   Serial.println(httpCode);   //Print HTTP return code
//   Serial.println(F("D40"));
//   Serial.println(payload);    //Print request response payload

//   client.end();  //Close connection
// }


// void sendPost_V1()
// // send data as POST to another webserver
// // This examples is according to https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
// // and uses string class to build the message
// {
//   Serial.println(F("D050 sendPost()"));
//   String message="board=";
//   message += TXT_BOARDID;
//   message += F("&vcc=");
//   message += ESP.getVcc();
//   message += "&output1=";
//   message += "1";
//   message += "&output2=";
//   message += "2";
//   message += "&button1=";
//   message += "3";
  
//   HTTPClient http;
//   http.begin(sendHttpTo);
//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//   http.POST(message);
//   http.writeToStream(&Serial);
//   http.end();
// }

