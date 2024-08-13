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

//*************************************************
// include headers depending on BOARDTYPE
//*************************************************

#include <Arduino.h>
#include <timeserver.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>
#include <EvaluateSensor.h>

  #if (BOARDTYPE == ESP32)

  #include <WiFi.h>
  #include <ESP_Mail_Client.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

  //WebServer server(80); // an instance for the webserver
  extern WebServer server; // declare an instance for the webserver

#else // BOARDTYPE == ESP8266)
  // for Send-Mail
  #include <ESP_Mail_Client.h>

  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>       // Bonjour/multicast DNS, finds the device on network by name
  
  #include <WiFiClient.h>
  
  #include <ESP8266WebServer.h>  // for the webserver
  #include <ESP8266HTTPClient.h> // for the webclient https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
  #include <ArduinoOTA.h>        // OTA Upload via ArduinoIDE

  #include <NTPClient.h> // get time from timeserver
  #include <WiFiUdp.h>

  ESP8266WebServer server(80); // an instance for the webserver
 

#endif

int val_AHH;
int val_AH;
int val_AL;
// int val_ALL;

int simVal_AHH = 1;
int simVal_AH  = 1;
int simVal_AL  = 0;
// int simVal_ALL = 0;

int firstRun = 1;

int debugLevelSwitches=0; // default off

/* =======================================*/
void handleNotFound() {
/* =======================================*/
  // digitalWrite(builtin_led, 1);
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
  // digitalWrite(builtin_led, 0);
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

  //************************************************ */
  // display stat of the relais input signals
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

  message += F("</pre></article>");

  // ------------- end of display relais

  //-------------------------------------------------
  //  Prepre and send message, depending on alarmState
  //------------------------------------------------
  message += F("<article>"
               "<h2>Auswertung Wasserstand</h2>");

  if (alarmState == 5) 
    message += F("<message_err> Achtung Hochwasser -- Pumpe einschalten <br>Wasserstand &gt " Level_AHH_Str "<br></message_err>");

  else if (alarmState == 4)
    message += F("<message_warn> Wasserstand ist zwischen " Level_AH_Str" und " Level_AHH_Str "<br></message_warn>");
  
  else if (alarmState == 3)
    message += F("<message_ok> Wasserstand ist zwischen " Level_AL_Str " und " Level_AH_Str " <br></message_ok>");

  else if (alarmState == 2)
    message += F("<message_ok>   Wasserstand &lt; " Level_AL_Str "<br></message_ok>");
  else
    message += F("<message_ok>   Wasserstand &lt; " Level_AL_Str "<br>Alarmstate 1/0 <br></message_ok>");

  // print when a new value arrives
  message += F("<br><br>  Zeit: ");
  message += formattedTime;
  message += F("   Wasserstand aktuell: ");
  message += myValueFilteredAct;

  //-----------------------------------------------------------------------------------------
  // End of show message depending of alarmstate
  message += F("</article>");

  //-----------------------------------------------------------------------------------------
  // Simulation
  message += F("<article>\n"
  //-----------------------------------------------------------------------------------------
  "<h2>Simulation</h2>\n" 
  "<p>Zum Einschalten der Simulation klicke auf den obersten Schalter "
  "<br>rot : Debug ein"
  "<br><br>Um die Relais Eingänge zu schalten, klicke auf den entsprechenden Schalter<br>"
  "<br>AHH / AH : rot   --> überschritten"
  "<br>AHH / AH : grau  --> unterschritten"
  "<br>    / AL : grün  --> unterschritten"
  "<br>    / AL : grau  --> überschritten"
  "</p>");
  if (debugLevelSwitches == 1) // simulation aktiv = rot
  {
    message += F("<p class='on_red'><a href='c.php?toggle=a' target='i'>Debug</a></p>\n");
  } else {
    message += F("<p class='off'><a href='c.php?toggle=a' target='i'>Debug</a></p>\n");
  }
  if (simVal_AHH == 0)  // low active => 0 = on = rot = higher than AHH
  {
  message += F("<p class='on_red'><a href='c.php?toggle=1' target='i'>AHH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=1' target='i'>AHH</a></p>\n");
  }
  if (simVal_AH == 0) // low active => higher than AH
  {
  message += F("<p class='on_red'><a href='c.php?toggle=2' target='i'>AH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=2' target='i'>AH</a></p>\n");
  }
  if (simVal_AL == 0)  // low active => 0 = on = green => lower! than
  {
  message += F("<p class='on_green'><a href='c.php?toggle=3' target='i'>AL</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=3' target='i'>AL</a></p>\n");
  }

  message += F("<p class='off'><a href='c.php?toggle=5' target='i'>LED</a></p>\n"
  "<iframe name='i' style='display:none' ></iframe>\n" // hack to keep the button press in the window
  //-----------------------------------------------------------------------------------------
  // end of simulation
  "</article>\n");

  // // add slider for waterlevel
  // message += F("<article>\n"
  // "<h2>Slider</h2>\n" 
  // "<div class='slidecontainer'>\n"
  // "<input type='range' min='1' max='100' value='50' class='slider' id='myRange'>\n"
  // "</div>");

  addBottom(message);
  server.send(200, "text/html", message);
}

void handleSlider()
{
  String message;
  addTop(message);

  message += F(

               "<!DOCTYPE html>"
               "<html>"
               "<head>"
               "<meta name='viewport' content='width=device-width, initial-scale=1'>"
               "<style>"
               ".slidecontainer {"
               "  width: 100%;"
               "}"
               ""
               ".slider {"
               "  -webkit-appearance: none;"
               "  width: 100%;"
               "  height: 25px;"
               "  background: #d3d3d3;"
               "  outline: none;"
               "  opacity: 0.7;"
               "  -webkit-transition: .2s;"
               "  transition: opacity .2s;"
               "}"
               ""
               ".slider:hover {"
               "  opacity: 1;"
               "}"
               ""
               ".slider::-webkit-slider-thumb {"
               "  -webkit-appearance: none;"
               "  appearance: none;"
               "  width: 25px;"
               "  height: 25px;"
               "  background: #04AA6D;"
               "  cursor: pointer;"
               "}"
               ""
               ".slider::-moz-range-thumb {"
               "  width: 25px;"
               "  height: 25px;"
               "  background: #04AA6D;"
               "  cursor: pointer;"
               "}"
               "</style>"
               "</head>"
               "<body>"
               ""
               "<h1>Custom Range Slider</h1>"
               "<p>Drag the slider to display the current value.</p>"
               ""
               "<div class='slidecontainer'>"
               "  <input type='range' min='1' max='100' value='50' class='slider' id='myRange'>"
               "  <p>Value: <span id='demo'></span></p>"
               "</div>"
               ""
               "<script>"
               "var slider = document.getElementById('myRange');"
               "var output = document.getElementById('demo');"
               "output.innerHTML = slider.value;"
               ""
               "slider.oninput = function() {"
               "  output.innerHTML = this.value;"
               "}"
               "</script>"
               ""
               "</body>"
               "</html>");

              addBottom(message);
              server.send(200, "text/html", message);
};

//=======================================================================================


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
  message.reserve(35000);

  String formattedTimeL= "";
  String formattedDateL= "";

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

      formatDateAndTime(formattedTimeL, formattedDateL, ringTime[rdRingPtr]);
      message += formattedTimeL;

      message += "  ";
      message += ringValue[rdRingPtr];
      message += "  ";
      message += ringADC[rdRingPtr];
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
      
      formatDateAndTime(formattedTimeL, formattedDateL, ringLongtermTime[rdLongtermRingPtr]);
      message += formattedTimeL;
      
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
  graphXValues.reserve(13000);

  String graphYValues = "";
  graphYValues.reserve(4000);

  String formattedTimeL= "";
  String formattedDateL= "";

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
        formatDateAndTime(formattedTimeL, formattedDateL, ringTime[rdRingPtr]);
        graphXValues += formattedTimeL;

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

    Serial.print("graphXValues: "); 
    Serial.println(graphXValues.length());
    Serial.print("graphYValues: "); 
    Serial.println(graphYValues.length());


  String message;
  message.reserve(20000);
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
  message += Level_AH;
  message += "\",0,";
  message += noValues;
  message +=",1);";

  message += "const graphYlevelErro = [];";
  message += "generateErroData(\"";
  message += Level_AHH;
  message += "\",0,";
  message += noValues;
  message +=",1);";
  

  message +=  "new Chart(\"Shortterm_chart\", {";
  message +=  " type: \"line\",";
  message +=  " data: {";
  message +=  "   labels: xValues,";
  message +=  "   datasets: [{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 1,";
  message +=  "     pointRadius: 1,";
  message +=  "     backgroundColor: \"rgba(0,0,255,1.0)\",";
  message +=  "     borderColor: \"rgba(0,0,255,0.5)\",";
  message +=  "     label: \"Wasserstand [cm]\",";
  message +=  "     data: yValues";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     pointRadius: 0,";
  message +=  "     backgroundColor: \"rgba(0,255,0,0)\",";
  message +=  "     borderColor: \"rgba(0,255,0,0.3)\",";
  message +=  "     label: \"Warnschwelle: ";
  message +=  Level_AH;
  message +=  " cm\",";
  message +=  "     data: graphYlevelWarn";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     pointRadius: 0,";
  message +=  "     backgroundColor: \"rgba(255,0,0,0)\",";
  message +=  "     borderColor: \"rgba(255,0,0,0.3)\",";
  message +=  "     label: \"Alarmschwelle: ";
  message +=  Level_AHH;
  message +=  " cm\",";
  message +=  "     data: graphYlevelErro";
  message +=  "   }]";
  message +=  " },";
  message +=  " options: {";
  message +=  "   title: {";
  message +=  "   display: false,";
  message +=  "   text: \"Wasserstand in cm\"";
  message +=  "   },";
  message +=  "   legend: {display: true, text: \"Wasserstand \"},";
  message +=  "   scales: {";
  message +=  "     yAxes: [{ticks: {min: 100, max:200}}]";
  message +=  "   }";
  message +=  " }";
  message +=  " });";
  message +=  " function generateWarnData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       ;";
  message +=  "       graphYlevelWarn.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  " function generateErroData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       ;";
  message +=  "       graphYlevelErro.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  "</script></body>";
            
  addBottom(message);

  Serial.print("message: "); 
  Serial.println(message.length());

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
  graphLongtermXValues.reserve(2000);

  String graphLongtermYValues = "";
  graphLongtermYValues.reserve(500);

  String formattedTimeL= "";
  String formattedDateL= "";

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

        formatDateAndTime(formattedTimeL, formattedDateL, ringLongtermTime[rdLongtermRingPtr]);
        graphLongtermXValues += formattedTimeL;

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

  Serial.print("graphLongtermXValues: "); 
  Serial.println(graphLongtermXValues.length());
  Serial.print("graphLongtermYValues: "); 
  Serial.println(graphLongtermYValues.length());


  
  String message;
  message.reserve(5000);

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
  message += Level_AH;
  message += "\",0,";
  message += noValues;
  message +=",1);";

  message += "const graphYlevelErro = [];";
  message += "generateErroData(\"";
  message += Level_AHH;
  message += "\",0,";
  message += noValues;
  message +=",1);";
  

  message +=  "new Chart(\"Longterm_chart\", {";
  message +=  " type: \"line\",";
  message +=  " data: {";
  message +=  "   labels: xValues,";
  message +=  "   datasets: [{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 1,";
  message +=  "     pointRadius: 2,";
  message +=  "     backgroundColor: \"rgba(0,0,255,1.0)\",";
  message +=  "     borderColor: \"rgba(0,0,255,0.5)\",";
  message +=  "     label: \"Wasserstand [cm]\",";
  message +=  "     data: yValues";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     pointRadius: 0,";
  message +=  "     backgroundColor: \"rgba(0,255,0,0)\",";
  message +=  "     borderColor: \"rgba(0,255,0,0.3)\",";
  message +=  "     label: \"Warnschwelle: ";
  message +=  Level_AH;
  message +=  " cm\",";
  message +=  "     data: graphYlevelWarn";
  message +=  "     },{";
  message +=  "     fill: false,";
  message +=  "     lineTension: 0,";
  message +=  "     pointRadius: 0,";
  message +=  "     backgroundColor: \"rgba(255,0,0,0)\",";
  message +=  "     borderColor: \"rgba(255,0,0,0.3)\",";
  message +=  "     label: \"Alarmschwelle: ";
  message +=  Level_AHH;
  message +=  " cm\",";
  message +=  "     data: graphYlevelErro";
  message +=  "   }]";
  message +=  " },";
  message +=  " options: {";
  message +=  "   title: {";
  message +=  "   display: false,";
  message +=  "   text: \"Wasserstand in cm\"";
  message +=  "   },";
  message +=  "   legend: {display: true, text: \"Wasserstand \"},";
  message +=  "   scales: {";
  message +=  "     yAxes: [{ticks: {min: 100, max:200}}]";
  message +=  "   }";
  message +=  " }";
  message +=  " });";
  message +=  " function generateWarnData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       ;";
  message +=  "       graphYlevelWarn.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  " function generateErroData(value, i1, i2, step = 1) {";
  message +=  "     for (let x = i1; x <= i2; x += step) {";
  message +=  "       ;";
  message +=  "       graphYlevelErro.push(eval(value));";
  message +=  "     }";
  message +=  "   }";
  message +=  "</script></body>";

  addBottom(message);

  Serial.print("message: "); 
  Serial.println(message.length());

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
              "p{color: black;}"
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
              ".on_red, .on_green, .off{margin-top:0;margin-bottom:0.2em;margin-left:4em;font-size:1.4em;border-style:solid;border-radius:10px;border-style:outset;width:5em;height:1.5em;text-decoration:none;text-align:center}"
              ".off{color:black;background-color:gray;border-color:grey}"
              ".on_red{background-color:red;border-color:red}"
              ".on_green{background-color:green;border-color:green}"
              "message_ok  {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:green ;width:19em;text-align:center}"
              "message_warn{color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:orange;width:19em;text-align:center}"
              "message_err {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:red   ;width:19em;text-align:center}"
              // //=========================================================================
              // ".slidecontainer {"
              // "  width: 100%; /* Width of the outside container */"
              // "}"
              // ""
              // "/* The slider itself */"
              // ".slider {"
              // "  -webkit-appearance: none;  /* Override default CSS styles */"
              // "  appearance: none;"
              // "  width: 100%; /* Full-width */"
              // "  height: 25px; /* Specified height */"
              // "  background: #d3d3d3; /* Grey background */"
              // "  outline: none; /* Remove outline */"
              // "  opacity: 0.7; /* Set transparency (for mouse-over effects on hover) */"
              // "  -webkit-transition: .2s; /* 0.2 seconds transition on hover */"
              // "  transition: opacity .2s;"
              // "}"
              // ""
              // "/* Mouse-over effects */"
              // ".slider:hover {"
              // "  opacity: 1; /* Fully shown on mouse-over */"
              // "}"
              // ""
              // "/* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */"
              // ".slider::-webkit-slider-thumb {"
              // "  -webkit-appearance: none; /* Override default look */"
              // "  appearance: none;"
              // "  width: 25px; /* Set a specific slider handle width */"
              // "  height: 25px; /* Slider handle height */"
              // "  background: #04AA6D; /* Green background */"
              // "  cursor: pointer; /* Cursor on hover */"
              // "}"
              // ""
              // ".slider::-moz-range-thumb {"
              // "  width: 25px; /* Set a specific slider handle width */"
              // "  height: 25px; /* Slider handle height */"
              // "  background: #04AA6D; /* Green background */"
              // "  cursor: pointer; /* Cursor on hover */"
              // "}"
              // ""
              //=========================================================================
              
              );
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
  // message += (F(",\"val_ALL\":"));
  // message += digitalRead(GPin_ALL);
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

// /* =======================================*/
// // Create a dynamic range slider to display the current value, with JavaScript:
// void handleSliderJs()
// {
//   /* =======================================*/
//   String message;
//   message += F("const url ='json';\n"
//                "var slider = document.getElementById('myRange');"
//                "var output = document.getElementById('demo');"
//                "output.innerHTML = slider.value; // Display the default slider value"

//                "// Update the current slider value (each time you drag the slider handle)"
//                "slider.oninput = function() {"
//                "  output.innerHTML = this.value;"
//                "}");

//   server.send(200, "text/javascript", message);
// }


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
    }
    if (server.arg(0) == "1") // the value for that parameter
    {
      Serial.println(F("D232 toggle ahh"));
      if (simVal_AHH == 1) {simVal_AHH = 0;} else {simVal_AHH = 1;}
      digitalWrite(GPin_AHH, simVal_AHH);
    }
    if (server.arg(0) == "2") // the value for that parameter
    {
      Serial.println(F("D232 toggle ah"));
      if (simVal_AH == 1) {simVal_AH = 0;} else {simVal_AH = 1;}
      digitalWrite(GPin_AH, simVal_AH);
    }
    if (server.arg(0) == "3") // the value for that parameter
    {
      Serial.println(F("D232 toggle al"));
      if (simVal_AL == 1) {simVal_AL = 0;} else {simVal_AL = 1;}
      digitalWrite(GPin_AL, simVal_AL);
    }
    if (server.arg(0) == "4") // the value for that parameter
    {
      Serial.println(F("D232 toggle all"));
      // if (simVal_ALL == 1) {simVal_ALL = 0;} else {simVal_ALL = 1;}
      // digitalWrite(GPin_AHH, simVal_ALL);
      // Serial.println(simVal_ALL);
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

  setPegelforSimulation();

}


//}
/*
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
========== end of including server.cpp
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
*/
