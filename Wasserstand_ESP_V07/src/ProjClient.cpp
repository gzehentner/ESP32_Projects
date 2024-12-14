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
  #include <WiFiClient.h>
  #include <HTTPClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

#else // BOARDTYPE == ESP8266)
   #include <ESP_Mail_Client.h>
   #include <ESP8266HTTPClient.h>  // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
#endif

int errCnt_communication = 0;

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

  strcat(message, "&pump1_op=");
  itoa(pump1_op, val, 10);
  strcat(message, val);

  strcat(message, "&pump2_op=");
  itoa(pump2_op, val, 10);
  strcat(message, val);

  strcat(message, "&epochTime=");
  itoa(epochTime, val, 10);
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

  // error handling
  if (httpCode<0) {

    // write to file
    String errMessage = "";
    errMessage =  currentDate + " - " + formattedTime + " - " +  " httpCode = " +  httpCode + "\n";
  //  appendFile("/error.log", errMessage.c_str());

    errCnt_communication++;

    if (errCnt_communication > ERR_CNT_COMMUNICATION)
    {
      // reset ESP8266
      errMessage = currentDate + " - " + formattedTime + " - " +  "client connection error - restart triggered\n";
    //  appendFile("/error.log", errMessage.c_str());
      ESP.restart();
    }
  } else {
    // reset to zero, when communication is running again
    errCnt_communication = 0;
  }
}
