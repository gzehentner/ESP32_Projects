//* *******************************************************************
//   Webserver
//
//   How it all works together:
//
//   page   0.htm         includes javascript j.js
//   script j.js          the javascript requests the JSON
//          json          returns data as JSON
//   css    f.css         css for all on flash (program) memory
//   php                  not really php - only a resource doing actions and not returning content (just http header 204 ok)
//   ***************************************************************** */
//

#include <Arduino.h>
#include <timeserver.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>

#if (BOARDTYPE == ESP32)
  // for Send-Mail
  // #include <ESP_Mail_Client.h>

  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

  //WebServer server(80); // an instance for the webserver
  extern WebServer server; // declare an instance for the webserver

#else // BOARDTYPE == ESP8266)
  // for Send-Mail
  #include <ESP_Mail_Client.h>

  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>  // for the webserver
  #include <ESP8266HTTPClient.h> // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
  #include <ESP8266mDNS.h>       // Bonjour/multicast DNS, finds the device on network by name
  #include <ArduinoOTA.h>        // OTA Upload via ArduinoIDE

  #include <NTPClient.h> // get time from timeserver
  #include <WiFiUdp.h>

  ESP8266WebServer server(80); // an instance for the webserver

#endif

int val_AHH;
int val_AH;
int val_AL;
int val_ALL;

int simVal_AHH = 0;
int simVal_AH  = 0;
int simVal_AL  = 0;
int simVal_ALL = 0;

int firstRun = 1;

int debugLevelSwitches=0;

String formatTime(unsigned long rawTime) {
  
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

// void handleRoot() {
//   digitalWrite(builtin_led, 1);
//   char temp[400];
//   int sec = millis() / 1000;
//   int min = sec / 60;
//   int hr = min / 60;

//   snprintf(temp, 400,

//            "<html>\
//   <head>\
//     <meta http-equiv='refresh' content='5'/>\
//     <title>ESP32 Demo</title>\
//     <style>\
//       body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
//     </style>\
//   </head>\
//   <body>\
//     <h1>Hello from ESP32!</h1>\
//     <p>Uptime: %02d:%02d:%02d</p>\
//     <img src=\"/test.svg\" />\
//   </body>\
// </html>",

//            hr, min % 60, sec % 60
//           );
//   server.send(200, "text/html", temp);
//   digitalWrite(builtin_led, 0);
// }

/* =======================================*/
void handleNotFound() {
/* =======================================*/
  digitalWrite(builtin_led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(builtin_led, 0);
}

// void drawGraph() {
//   String out = "";
//   char temp[100];
//   out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
//   out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
//   out += "<g stroke=\"black\">\n";
//   int y = rand() % 130;
//   for (int x = 10; x < 390; x += 10) {
//     int y2 = rand() % 130;
//     sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
//     out += temp;
//     y = y2;
//   }
//   out += "</g>\n</svg>\n";

//   server.send(200, "image/svg+xml", out);
// }

// /* =======================================*/
// void handleNotFound()
// {
//   /* =======================================*/
//   // Output a "404 not found" page. It includes the parameters which comes handy for test purposes.
//   Serial.println(F("D015 handleNotFound()"));
//   String message;
//   message += F("404 - File Not Found\n"
//                "URI: ");
//   message += server.uri();
//   message += F("\nMethod: ");
//   message += (server.method() == HTTP_GET) ? "GET" : "POST";
//   message += F("\nArguments: ");
//   message += server.args();
//   message += F("\n");
//   for (uint8_t i = 0; i < server.args(); i++)
//   {
//     message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//   }
//   server.send(404, "text/plain", message);
// }

// /* =======================================*/
// void handle204()
// /* =======================================*/
// {
//   server.send(204); // this page doesn't send back content
// }

/* =======================================*/
/* add a header  to each page including refs for all other pages */
void addTop(String &message)
/* =======================================*/
{
  message = F("<!DOCTYPE html>"
              "<html lang='en'>"
              "<head>"
              "<title>Wasserstand-Messung</title>"
              "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
              "<meta name=\"viewport\" content=\"width=device-width\">"
              "<link rel='stylesheet' type='text/css' href='/f.css'>"
              "<script src='j.js'></script>"
              "</head>");
  message += F("<body>");
  message += F("<header><h1>" TXT_BOARDNAME " - Board " TXT_BOARDID "</h1>");
  message += F("<nav> <a href=\"/\">[Home]</a> <a href=\"filtered.htm\">[Value History]</a>" 
               "<a href=\"longterm_graph.htm\">[Longterm Graph]</a>" "<a href=\"graph.htm\">[Shorterm Graph]</a> </nav></header>"
               "<main>");
}

/* =======================================*/
/* The footer will display the uptime, the IP-address the version of the sketch and the compile date/time */
/* add code to calculate the uptime */
void addBottom(String &message)
{
  /* =======================================*/

  message += F("</main>"
               "<footer><p>");
  message += F("<p>Actual Date and Time: ");

  message += currentDate;
  message += F(" -- ");
  message += formattedTime;
  message += F("<br>");

  if (seconds_since_startup > 604800)
  {
    message += F("<span id='week'>");
    message += ((seconds_since_startup / 604800) % 52);
    message += F("</span> weeks ");
  }
  if (seconds_since_startup > 86400)
  {
    message += F("<span id='day'>");
    message += ((seconds_since_startup / 86400) % 7);
    message += F("</span> days ");
  }
  if (seconds_since_startup > 3600)
  {
    message += F("<span id='hour'>");
    message += ((seconds_since_startup / 3600) % 24);
    message += F("</span> hours ");
  }

  message += F("<span id='min'>");
  message += ((seconds_since_startup / 60) % 60);
  message += F("</span> minutes ");

  message += F("<span id='sec'>");
  message += (seconds_since_startup % 60);
  message += F("</span> seconds since startup | Version " VERSION " | IP: ");
  message += WiFi.localIP().toString();
  message += F(" | " __DATE__ " " __TIME__ "</p></footer></body></html>");
  server.send(200, "text/html", message);
}

// the html output
// finally check your output if it is valid html: https://validator.w3.org
// *** HOME ***  0.htm
/* =======================================*/
/* main page of this application:
 *   - display water level as raw data
 *   - diaplay actual warn or alarm satus
 */
void handlePage()
/* =======================================*/
{
  String message;
  addTop(message);

  message += F("<article>"
               "<h2>Wasserstand Zehentner Teisendorf</h2>" // here you write your html code for your homepage. Let's give some examples...
               "<p>Hier kann man den aktuellen Wasserstand in der Regenwasser-Zisterne  "
               "von Georg Zehentner, Streiblweg 19, Teisendorf ablesen.<br> "
               " Bei Überschreiten des Höchststand muss eine Pumpe aktiviert werden</p>"
               "</article>");

  message += F("<article>"
               "<h2>Rohdaten</h2><pre>");

  // input signals are low active
  message += F("Level Alarm   high: <span id='val_AHH'>");
  if (val_AHH == 0)
  {
    message += F("active");
  }
  else
  {
    message += F("--");
  }
  message += F("</span><br>");
  message += F("Level Warning high: <span id='val_AH' >");
  if (val_AH == 0)
  {
    message += F("active");
  }
  else
  {
    message += F("--");
  }
  message += F("</span><br>");
  message += F("Level Warning low:  <span id='val_AL' >");
  if (val_AL == 0)
  {
    message += F("active");
  }
  else
  {
    message += F("--");
  }
  message += F("</span><br>");
  message += F("Level Alarm   low:  <span id='val_ALL'>");
  if (val_ALL == 0)
  {
    message += F("active");
  }
  else
  {
    message += F("--");
  }
  message += F("</span><br>");
  message += F("</pre></article>");

  message += F("<article>"
               "<h2>Auswertung Wasserstand</h2>");
  if (alarmState == 5)
  {
    message += F("<message_err> Achtung Hochwasser -- Pumpe einschalten <br>Wasserstand &gt ");
    message += Level_AHH;
    message += F("<br></message_err>");
  }
  else if (alarmState == 4)
  {
    message += F("<message_warn> Wasserstand ist zwischen ");
    message += Level_AH;
    message += F(" und ");
    message += Level_AHH;
    message += F("<br></message_warn>");
  }
  else if (alarmState == 3)
  {
    message += F("<message_ok> Wasserstand ist zwischen ");
    message += Level_AL;
    message += F(" und ");
    message += Level_AH;
    message += F("<br></message_ok>");
  }
  else if (alarmState == 2)
  {
    message += F("<message_ok> Wasserstand ist zwischen ");
    message += Level_ALL;
    message += F(" und ");
    message += Level_AL;

    message += F("<br></message_ok>");
  }
  else if (alarmState == 1)
  {
    message += F("<message_ok>   Wasserstand &lt; ");
    message += Level_ALL;
    message += F("<br></message_ok>");
  }

  // print when a new value arrieves
  message += F("<br><br>  Zeit: ");
  message += formattedTime;
  message += F("   Wasserstand aktuell: ");
  message += myValueFilteredAct;

  message += F("</article>");

  message += F("<article>\n"
  "<h2>Simulate</h2>\n" 
  "<p>Enable Simulation and switch relais outputs "
  "Click to toggle the output.</p>\n");
  if (debugLevelSwitches) 
  {
    message += F("<p class='on'><a href='c.php?toggle=a' target='i'>Debug</a></p>\n");
  } else {
    message += F("<p class='off'><a href='c.php?toggle=a' target='i'>Debug</a></p>\n");
  }
  if (simVal_AHH) 
  {
  message += F("<p class='on'><a href='c.php?toggle=1' target='i'>AHH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=1' target='i'>AHH</a></p>\n");
  }
  if (simVal_AH) 
  {
  message += F("<p class='on'><a href='c.php?toggle=2' target='i'>AH </a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=2' target='i'>AH </a></p>\n");
  }
  if (simVal_AL) 
  {
  message += F("<p class='on'><a href='c.php?toggle=3' target='i'>AL </a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=3' target='i'>AL </a></p>\n");
  }
  if (simVal_ALL) 
  {
  message += F("<p class='on'><a href='c.php?toggle=4' target='i'>ALL</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=4' target='i'>ALL</a></p>\n");
  }
  message += F("<p class='off'><a href='c.php?toggle=5' target='i'>LED</a></p>\n"
  "<iframe name='i' style='display:none' ></iframe>\n" // hack to keep the button press in the window
  "</article>\n");

  addBottom(message);
  server.send(200, "text/html", message);
}


/* =======================================*/
/* print value history*/
void handleListFiltered()
/* =======================================*/
{
  // what string length do we need?
  //   listing shortterm 
  //   - per line 25
  //   - 100 lines
  //   listing longterm
  //   - per line 25
  //   - 120 lines
  //  sum: 550 character without header and footer
  // 1 - message length: 513
  // 2 - message length: 3909
  
  String message = "";

  int iLine = iRingValueMax;

 addTop(message);

  Serial.print("1 - message length: "); 
  Serial.println(message.length());

  //---------------------
  // shortterm values
  //---------------------
  message += F("<article>"
               "<h2>Wasserstand Zehentner Teisendorf</h2>" 
               "<p>Shortterm values<br>Index ");
  message += filterCnt;             
  message += "<br> </p>";

  // create header of table
  message += "<pre>rdRingPtr  Time     Value ADC <br>";
  
  // read out ringbuffer and display values, but not more than maxLines lines
  for ( rdRingPtr = wrRingPtr+1; (rdRingPtr != wrRingPtr); ){
    // print only the last lines
    if (iLine <= maxLines) {
      message += rdRingPtr;
      message += "       ";
      message += formatTime(ringTime[rdRingPtr]);
      message += "  ";
      message += ringValue[rdRingPtr];
      // message += ringADC[rdRingPtr];
      message += "<br>";
    }

    if (rdRingPtr<iRingValueMax) {
      rdRingPtr++;
    } else {
      rdRingPtr = 0;
    }
    iLine--;
    
  }
  message += F("</pre></article>");

  //---------------------
  // longterm values
  //---------------------
  message += F("<article>"
              "<h2>Wasserstand Zehentner Teisendorf</h2>" 
              "<p>Longtterm values<br>Index ");
  message += filterCnt;             
  message += "<br> </p>";

  // create header of table
  message += "<pre>rdRingPtr  Time     Value <br>";
  
  // read out ringbuffer and display values, but not more than maxLines lines
  iLine = iLongtermRingValueMax;
  for ( rdLongtermRingPtr = wrLongtermRingPtr+1; (rdLongtermRingPtr != wrLongtermRingPtr); ){

    // print only the last lines
    if (iLine <= maxLines) {
      message += rdLongtermRingPtr;
      message += "       ";
     message += formatTime(ringLongtermTime[rdLongtermRingPtr]);
  message += "  ";
      message += ringLongtermValue[rdLongtermRingPtr];
       message += "<br>";
    }
    if (rdLongtermRingPtr<iLongtermRingValueMax) {
      rdLongtermRingPtr++;
    } else {
      rdLongtermRingPtr = 0;
    }
    iLine--;

  }
  message += F("</pre></article><br>");

  addBottom(message);
  server.send(200, "text/html", message);
  
  Serial.print("2 - message length: "); 
  Serial.println(message.length());
}
/* =======================================*/
/* print both graph longterm and shortterm*/
void handleGraph()
/* =======================================*/
{
  String graphXValues = "";     // values for graph (displayed)
  String graphYValues = "";
  int noValues = 0;
  int iPoint = iRingValueMax;

  // prepare values for graph
  graphXValues  = "const xValues = [";
  graphYValues  = "const yValues = [";


  // read out ringbuffer and create the vector to display as graph
  for ( rdRingPtr = wrRingPtr+1; rdRingPtr != wrRingPtr; ){
    
    // if there is a valid time set (time="" means there is no value written since last startup)
    if (ringTime[rdRingPtr] != 0) {
      if (iPoint <= maxPoints) {
        // fill X values time
        graphXValues += "\"";
        graphXValues += formatTime(ringTime[rdRingPtr]);
        graphXValues += "\", ";
        // take value and place it to the string for graph
        graphYValues += ringValue[rdRingPtr];
        graphYValues += ", ";
        noValues ++;
      }
    }
    if (rdRingPtr<iRingValueMax) {
      rdRingPtr++;
    } else {
      rdRingPtr = 0;
    }
    iPoint--;  
    }
  
    // enclose the generated strings with necessary brakets
    firstRun = 0;
    graphXValues += "];";
    graphYValues += "];";


  String message;
  addTop(message);


  message += F("<article>"
               "<h2>Wasserstand Zehentner Teisendorf</h2>" 
               "<p>Line Graph -- Shortterm values<br> </p> "); 

  message += "<br>filterCnt= ";
  message += filterCnt;

  // print when a new value arrives
  message += F("<br><br>  Zeit: ");
  message += formattedTime;
  message += F("   Wasserstand aktuell: ");
  message += myValueFilteredAct;
  message += "</article>";

  message += "<script"
               " src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js\">"
               "</script>";
  
  message += "<body><canvas id=\"Shortterm_chart\" style=\"width:100%;max-width:700px\"></canvas>";
  
  message += "<script>";
  message += graphXValues; 
  message += graphYValues; 

  message += "const graphYlevelWarn = [];";
  message += "generateWarnData(\"";
  message += Level_AH*10;
  message += "\",0,";
  message += noValues;
  message +=",1);";

  message += "const graphYlevelErro = [];";
  message += "generateErroData(\"";
  message += Level_AHH*10;
  message += "\",0,";
  message += noValues;
  message +=",1);";
  

  message +=  "new Chart(\"Shortterm_chart\", {";
  message +=  " type: \"line\",";
  message +=  " data: {";
  message +=  "   labels: xValues,";
  message +=  "   datasets: [{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(0,0,255,1.0)\",";
  message +=  "     borderColor: \"rgba(0,0,255,0.5)\",";
  message +=  "     label: \"Wasserstand [mm]\",";
  message +=  "     data: yValues";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(0,255,0,0)\",";
  message +=  "     borderColor: \"rgba(0,255,0,0.3)\",";
  message +=  "     label: \"Warnschwelle: ";
  message +=  Level_AH;
  message +=  "0 mm\",";
  message +=  "     data: graphYlevelWarn";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(255,0,0,0)\",";
  message +=  "     borderColor: \"rgba(255,0,0,0.3)\",";
  message +=  "     label: \"Alarmschwelle: ";
  message +=  Level_AHH;
  message +=  "0 mm\",";
  message +=  "     data: graphYlevelErro";
  message +=  "   }]";
  message +=  " },";
  message +=  " options: {";
  message +=  "   title: {";
  message +=  "   display: false,";
  message +=  "   text: \"Wasserstand in mm\"";
  message +=  "   },";
  message +=  "   legend: {display: true, text: \"Wasserstand \"},";
  message +=  "   scales: {";
  message +=  "     yAxes: [{ticks: {min: 1000, max:2000}}]";
  message +=  "   }";
  message +=  " }";
  message +=  " });";
  message +=  " function generateWarnData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       xValues.push(x);";
  message +=  "       graphYlevelWarn.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  " function generateErroData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       xValues.push(x);";
  message +=  "       graphYlevelErro.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  "</script></body>";
            
  addBottom(message);
  server.send(200, "text/html", message);

  graphXValues = "";                     // erase all values
  graphYValues = "";
    
}

/* =======================================*/
/* print graph longterm*/
void handleLongtermGraph()

/* =======================================*/
{
  String graphLongtermXValues = "";     // values for graph (displayed)
  String graphLongtermYValues = "";
  int noValues = 0;
  int iPoint = iLongtermRingValueMax;

  // prepare values for graph
  graphLongtermXValues  = "const xValues = [";
  graphLongtermYValues  = "const yValues = [";


  // read out ringbuffer and create the vector to display as graph
  for ( rdLongtermRingPtr = wrLongtermRingPtr+1; rdLongtermRingPtr != wrLongtermRingPtr; ){
    
    // if there is a valid time set (time="" means there is no value written since last startup)
    if (ringLongtermTime[rdLongtermRingPtr] != 0) {
      if (iPoint <= maxPoints) {

        // fill X values time
        graphLongtermXValues += "\"";
        graphLongtermXValues += formatTime(ringLongtermTime[rdLongtermRingPtr]);
        graphLongtermXValues += "\", ";
        // take value and place it to the string for graph
        graphLongtermYValues += ringLongtermValue[rdLongtermRingPtr];
        graphLongtermYValues += ", ";
        noValues ++;
      }   
    }
    if (rdLongtermRingPtr<iLongtermRingValueMax) {
      rdLongtermRingPtr++;
    } else {
      rdLongtermRingPtr = 0;
    }
    iPoint--;  

    }
  
    // enclose the generated strings with necessary brakets
    firstRun = 0;
    graphLongtermXValues += "];";
    graphLongtermYValues += "];";


  
  String message;
  addTop(message);


  message += F("<article>"
               "<h2>Wasserstand Zehentner Teisendorf</h2>" 
               "<p>Line Graph<br> - Longterm values </p> "); 

  message += "<br>filterCnt= ";
  message += filterCnt;

  // print when a new value arrives
  message += F("<br><br>  Zeit: ");
  message += formattedTime;
  message += F("   Wasserstand aktuell: ");
  message += myValueFilteredAct;
  message += "</article>";

  message += "<script"
               " src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js\">"
               "</script>";
  
  /* handle longterm */
  message += "<body><canvas id=\"Longterm_chart\" style=\"width:100%;max-width:700px\"></canvas>";

  message += "<script>";
  message += graphLongtermXValues; 
  message += graphLongtermYValues; 
  
  message += "const graphYlevelWarn = [];";
  message += "generateWarnData(\"";
  message += Level_AH*10;
  message += "\",0,";
  message += noValues;
  message +=",1);";

  message += "const graphYlevelErro = [];";
  message += "generateErroData(\"";
  message += Level_AHH*10;
  message += "\",0,";
  message += noValues;
  message +=",1);";
  

  message +=  "new Chart(\"Longterm_chart\", {";
  message +=  " type: \"line\",";
  message +=  " data: {";
  message +=  "   labels: xValues,";
  message +=  "   datasets: [{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(0,0,255,1.0)\",";
  message +=  "     borderColor: \"rgba(0,0,255,0.5)\",";
  message +=  "     label: \"Wasserstand [mm]\",";
  message +=  "     data: yValues";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(0,255,0,0)\",";
  message +=  "     borderColor: \"rgba(0,255,0,0.3)\",";
  message +=  "     label: \"Warnschwelle: ";
  message +=  Level_AH;
  message +=  "0 mm\",";
  message +=  "     data: graphYlevelWarn";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     backgroundColor: \"rgba(255,0,0,0)\",";
  message +=  "     borderColor: \"rgba(255,0,0,0.3)\",";
  message +=  "     label: \"Alarmschwelle: ";
  message +=  Level_AHH;
  message +=  "0 mm\",";
  message +=  "     data: graphYlevelErro";
  message +=  "   }]";
  message +=  " },";
  message +=  " options: {";
  message +=  "   title: {";
  message +=  "   display: false,";
  message +=  "   text: \"Wasserstand in mm\"";
  message +=  "   },";
  message +=  "   legend: {display: true, text: \"Wasserstand \"},";
  message +=  "   scales: {";
  message +=  "     yAxes: [{ticks: {min: 1000, max:2000}}]";
  message +=  "   }";
  message +=  " }";
  message +=  " });";
  message +=  " function generateWarnData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       xValues.push(x);";
  message +=  "       graphYlevelWarn.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  " function generateErroData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       xValues.push(x);";
  message +=  "       graphYlevelErro.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  "</script></body>";

  addBottom(message);
  server.send(200, "text/html", message);
  
  graphLongtermXValues = "";                     // erase all values
  graphLongtermYValues = "";

}

/* =======================================*/
void handleCss()
/* =======================================*/
{
  // output of stylesheet
  // this is a straight forward example how to generat a static page from program memory
  String message;
  message = F("*{font-family:sans-serif}"
              "body{margin:10px}"
              "h1, h2{color:white;background:" CSS_MAINCOLOR ";text-align:center}"
              "h1{font-size:1.2em;margin:1px;padding:5px}"
              "h2{font-size:1.0em}"
              "h3{font-size:0.9em}"
              "a{text-decoration:none;color:dimgray;text-align:center}"
              "main{text-align:center}"
              "article{vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:#E5E5E5;width:20em;text-align:left}" // if you don't like the floating effect (vor portrait mode on smartphones!) - remove display:inline-block
              "article h2{margin-top:0;padding-bottom:1px}"
              "section {margin-bottom:0.2em;clear:both;}"
              "table{border-collapse:separate;border-spacing:0 0.2em}"
              "th, td{background-color:#C0C0C0}"
              "button{margin-top:0.3em}"
              "footer p{font-size:0.8em;color:dimgray;background:silver;text-align:center;margin-bottom:5px}"
              "nav{background-color:silver;margin:1px;padding:5px;font-size:0.8em}"
              "nav a{color:dimgrey;padding:10px;text-decoration:none}"
              "nav a:hover{text-decoration:underline}"
              "nav p{margin:0px;padding:0px}"
              ".on, .off{color:white;margin-top:0;margin-bottom:0.2em;margin-left:4em;font-size:1.4em;border-style:solid;border-radius:10px;border-style:outset;width:5em;height:1.5em;text-decoration:none;text-align:center}"
              ".on{background-color:green;border-color:green}"
              ".off{background-color:red;border-color:red}"
              "message_ok  {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:green ;width:19em;text-align:center}"
              "message_warn{color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:orange;width:19em;text-align:center}"
              "message_err {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:red   ;width:19em;text-align:center}");
  server.send(200, "text/css", message);
}

/* =======================================*/
// Output: send data to browser as JSON
// after modification always check if JSON is still valid. Just call the JSON (json) in your webbrowser and check.
void handleJson()
{
  /* =======================================*/
  //  Serial.println(F("D268 requested json"));
  String message = "";
  message = (F("{\"ss\":")); // Start of JSON and the first object "ss":
  message += millis() / 1000;
  message += (F(",\"val_AHH\":"));
  message += digitalRead(GPin_AHH);
  message += (F(",\"val_AH\":"));
  message += digitalRead(GPin_AH);
  message += (F(",\"val_AL\":"));
  message += digitalRead(GPin_AL);
  message += (F(",\"val_ALL\":"));
  message += digitalRead(GPin_ALL);
  message += (F("}"));                           // End of JSON
  server.send(200, "application/json", message); // set MIME type https://www.ietf.org/rfc/rfc4627.txt
}

/* =======================================*/
// Output: a fetch API / JavaScript
// a function in the JavaScript uses fetch API to request a JSON file from the webserver and updates the values on the page if the object names and ID are the same
void handleJs()
{
  /* =======================================*/
  String message;
  message += F("const url ='json';\n"
               "function renew(){\n"
               " document.getElementById('sec').style.color = 'blue'\n" // if the timer starts the request, the second gets blue
               " fetch(url)\n"                                          // Call the fetch function passing the url of the API as a parameter
               " .then(response => {return response.json();})\n"
               " .then(jo => {\n"
               "   document.getElementById('sec').innerHTML = Math.floor(jo['ss'] % 60);\n" // example how to change a value in the HTML page
               "   for (var i in jo)\n"
               "    {if (document.getElementById(i)) document.getElementById(i).innerHTML = jo[i];}\n" // as long as the JSON name fits to the HTML id, the value will be replaced
               // add other fields here (e.g. the delivered JSON name doesn't fit to the html id
               // finally, update the runtime
               "   if (jo['ss'] > 60) { document.getElementById('min').innerHTML = Math.floor(jo['ss'] / 60 % 60);}\n"
               "   if (jo['ss'] > 3600) {document.getElementById('hour').innerHTML = Math.floor(jo['ss'] / 3600 % 24);}\n"
               "   if (jo['ss'] > 86400) {document.getElementById('day').innerHTML = Math.floor(jo['ss'] / 86400 % 7);}\n"
               "   if (jo['ss'] > 604800) {document.getElementById('week').innerHTML = Math.floor(jo['ss'] / 604800 % 52);}\n"
               "   document.getElementById('sec').style.color = 'dimgray';\n" // if everything was ok, the second will be grey again.
               " })\n"
               " .catch(function() {\n" // this is where you run code if the server returns any errors
               "  document.getElementById('sec').style.color = 'red';\n"
               " });\n"
               "}\n"
               "document.addEventListener('DOMContentLoaded', renew, setInterval(renew, ");
  message += ajaxIntervall * 1000;
  message += F("));");

  server.send(200, "text/javascript", message);
}

void handleCommand()
{

  int tempVal = 0;

  // receive command and handle accordingly
  Serial.println(F("D223 handleCommand"));
  for (uint8_t i = 0; i < server.args(); i++)
  {
    Serial.print(server.argName(i));
    Serial.print(F(": "));
    Serial.println(server.arg(i));
  }
  if (server.argName(0) == "toggle") // the parameter which was sent to this server
  {
    if (server.arg(0) == "a") // the value for that parameter
    {
      Serial.println(F("D232 toggle debug switch"));

      if (debugLevelSwitches == 1)
      { // toggle: if the pin was on - switch it of and vice versa
        debugLevelSwitches=0;
      }
      else
      {
        debugLevelSwitches=1;
      }
      Serial.print("debugLevelSwitch now ");
      Serial.println(debugLevelSwitches);
    }
    if (server.arg(0) == "1") // the value for that parameter
    {
      Serial.println(F("D232 toggle ahh"));

      if (simVal_AHH = digitalRead(GPin_AHH))
      { // toggle: if the pin was on - switch it of and vice versa
        digitalWrite(GPin_AHH, LOW);
      }
      else
      {
        digitalWrite(GPin_AHH, HIGH);
      }
    }
    if (server.arg(0) == "2") // the value for that parameter
    {
      Serial.println(F("D232 toggle ah"));
      if (simVal_AH  = digitalRead(GPin_AH))
      {
        digitalWrite(GPin_AH, LOW);
        Serial.println("AH is now low");
      }
      else
      {
        Serial.println("AH is now high");
        digitalWrite(GPin_AH, HIGH);
      }
    }
    if (server.arg(0) == "3") // the value for that parameter
    {
      Serial.println(F("D232 toggle al"));

      if (simVal_AL  = digitalRead(GPin_AL))
      { // toggle: if the pin was on - switch it of and vice versa
        digitalWrite(GPin_AL, LOW);
      }
      else
      {
        digitalWrite(GPin_AL, HIGH);
      }
    }
    if (server.arg(0) == "4") // the value for that parameter
    {
      Serial.println(F("D232 toggle all"));
      if (simVal_ALL = digitalRead(GPin_ALL))
      {
        digitalWrite(GPin_ALL, LOW);
      }
      else
      {
        digitalWrite(GPin_ALL, HIGH);
      }
    }
    if (server.arg(0) == "5") // the value for that parameter(led))
    {
      Serial.println(F("D232 toggle LED"));
      if (digitalRead(builtin_led))
      { // toggle: if the pin was on - switch it of and vice versa
        digitalWrite(builtin_led, LOW);
      }
      else
      {
        digitalWrite(builtin_led, HIGH);
      }
    }
  }
  else if (server.argName(0) == "CMD" && server.arg(0) == "RESET") // Example how to reset the module. Just send ?CMD=RESET
  {
    Serial.println(F("D238 will reset"));
    ESP.restart();
  }
  server.send(204, "text/plain", "No Content"); // this page doesn't send back content --> 204
}

//}
/*
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
========== end of including server.cpp
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
*/
