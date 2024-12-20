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
#include <MyLittleFSLib.h>
#include <stdio.h>
#include <string.h>

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

  //#include <NTPClient.h> // get time from timeserver
  #include <WiFiUdp.h>

  ESP8266WebServer server(80); // an instance for the webserver
 

#endif


int val_AHHH;
int val_AHH;
int val_AH;
int val_AL;
// int val_ALL;

//**************************************************************
//**************************************************************
// global variables to simulate remote board
//**************************************************************
int remoteBoardId;
int remoteLevelAct;
int remoteDebugLevelSwitches;
int remoteAHH;
int remoteAH;
int remoteAL;
int remoteLastMessage;
int remoteMessagesSucessfull;

int sendToClient = 0; // enable sending to client (develop system)
int useLiveMail  = 0; // to check the email send process, we want to use the live email address, because t-online is very slow

// **************************************************************************************************
// variables for simulation
// **************************************************************************************************
int debugLevelSwitches=0; // default off
int simVal_AHHH = 1;
int simVal_AHH  = 1;
int simVal_AH   = 1;
int simVal_AL   = 0;
int simError    = 0;    //  sim one failed sendPost (ProjClient.cpp)
int simReboot   = 0;    //  force reboot due to many failed transmissions to client
// int simVal_ALL = 0;

int firstRun = 1;
//int reloadDone = 1; // show if reload of handlePage is done

int time_steps = 0;


// **************************************************************************************************
void handleNotFound() {
// **************************************************************************************************
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
    message += String(" "); 
    message += String(server.argName(i)) ;
    message += ": " ;
    message += server.arg(i) ;
    message += "\n";
  }

  server.send(404, "text/plain", message);
  // digitalWrite(builtin_led, 0);
}


// **************************************************************************************************
/* add a header  to each page including refs for all other pages */
void addTop(String &message)
// **************************************************************************************************
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
  message += F("<nav>" 
                  "<a href=\"/\">[Home]</a> <a href=\"filtered.htm\">[Value History]</a>" 
                  "<a href=\"graph.htm\">[Shorterm Graph]</a>" 
                  "<a href=\"update\">[OTA update]</a>" 
                  "</nav></header>"
               "<main>");
}

// **************************************************************************************************
/* The footer will display the uptime, the IP-address the version of the sketch and the compile date/time */
/* add code to calculate the uptime */
void addBottom(String &message)
{
// **************************************************************************************************

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

// **************************************************************************************************
// **************************************************************************************************
// *** HOME ***  0.htm
/* =======================================*/
/* main page of this application:
 *   - display water level as raw data
 *   - diaplay actual warn or alarm satus
 */
void handlePage()
// **************************************************************************************************
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
  message += F("Level Alarm  very high: <span id='val_AHHH'>");
  if (val_AHHH == 0)
  {
    message += F("active");
  }
  else
  {
    message += F("--");
  }
  message += F("</span><br>");
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
  //  Prepare and send message, depending on alarmState
  //------------------------------------------------
  message += F("<article>"
               "<h2>Auswertung Wasserstand</h2>");

  if (alarmState == 6) 
    message += F("<message_err> Achtung extremer Starkregen -- beide Pumpen einschalten <br>Wasserstand &gt " Level_AHHH_Str "<br></message_err>");
  else if (alarmState == 5) 
    message += F("<message_err> Achtung Hochwasser -- Pumpe einschalten <br>Wasserstand &gt " Level_AHH_Str "<br></message_err>");

  else if (alarmState == 4)
    message += F("<message_warn> Wasserstand ist zwischen " Level_AH_Str" und " Level_AHH_Str "<br></message_warn>");
  
  else if (alarmState == 3)
    message += F("<message_ok> Wasserstand ist zwischen " Level_AL_Str " und " Level_AH_Str " <br></message_ok>");

  else if (alarmState == 2)
    message += F("<message_ok>   Wasserstand &lt; " Level_AL_Str "<br></message_ok>");
  else
    message += F("<message_ok>   Wasserstand &lt; " Level_AL_Str "<br>Alarmstate 1/0 <br></message_ok>");

  //------------------------------------------------
  // print when a new value arrives
  message += F("<br><br>  Zeit: ");
  message += formattedTime;
  message += F("   Wasserstand aktuell: ");
  message += myValueFilteredAct;

  //-----------------------------------------------------------------------------------------
  // End of show message depending of alarmstate
  message += F("</article>");
  message += F("<head>"
"    <title>Seite neu laden</title>                         "
"    <script>                                               "
"        function reloadPage() {                            "
"            if (!sessionStorage.getItem('reloaded')) {     "
"                sessionStorage.setItem('reloaded', 'true');"
"                setTimeout(function() {                    "
"                   location.reload();                      "
"                }, 200);                                   "
"            }                                              "
"        }                                                  "
"                                                           "
"        window.onload = function() {                       "
"            sessionStorage.removeItem('reloaded');         "
"            console.log('Die Seite wurde neu geladen.');  "
"        };                                                 "
"    </script>                                              "
"</head>                                                    "
"</html>                                                    ");



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
    message += F("<p class='on_red'><a href='c.php?toggle=a' target='i' onclick='reloadPage()' >Debug</a></p>\n");
  } else {
    message += F("<p class='off'><a href='c.php?toggle=a' target='i' onclick='reloadPage()'>Debug</a></p>\n");
  }
  
  if (simVal_AHHH == 0)  // low active => 0 = on = rot = higher than AHHH
  {
  message += F("<p class='on_red'><a href='c.php?toggle=0' target='i' onclick='reloadPage()'>AHHH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=0' target='i' onclick='reloadPage()'>AHHH</a></p>\n");
  }
  
  if (simVal_AHH == 0)  // low active => 0 = on = rot = higher than AHH
  {
  message += F("<p class='on_red'><a href='c.php?toggle=1' target='i' onclick='reloadPage()'>AHH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=1' target='i' onclick='reloadPage()'>AHH</a></p>\n");
  }
  if (simVal_AH == 0) // low active => higher than AH
  {
  message += F("<p class='on_red'><a href='c.php?toggle=2' target='i' onclick='reloadPage()'>AH</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=2' target='i' onclick='reloadPage()'>AH</a></p>\n");
  }
  if (simVal_AL == 0)  // low active => 0 = on = green => lower! than
  {
  message += F("<p class='on_green'><a href='c.php?toggle=3' target='i' onclick='reloadPage()'>AL</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=3' target='i' onclick='reloadPage()'>AL</a></p>\n");
  }
  
  if (simError == 1)  // 
  {
  message += F("<p class='on_red'><a href='c.php?toggle=6' target='i' onclick='reloadPage()'>Error</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=6' target='i' onclick='reloadPage()'>non error</a></p>\n");
  }
  if (simReboot == 1)  // 
  {
  message += F("<p class='on_red'><a href='c.php?toggle=7' target='i' onclick='reloadPage()'>Reboot</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=7' target='i' onclick='reloadPage()'>nRboot</a></p>\n");
  }
  if (useLiveMail == 1)  // 
  {
  message += F("<p class='on_green'><a href='c.php?toggle=8' target='i' onclick='reloadPage()'>LiveMail</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=8' target='i' onclick='reloadPage()'>DevMail</a></p>\n");
  }
  
  if (sendToClient == 1)  // 
  {
  message += F("<p class='on_green'><a href='c.php?toggle=5' target='i' onclick='reloadPage()'>Post</a></p>\n");
  } else {
  message += F("<p class='off'><a href='c.php?toggle=5' target='i' onclick='reloadPage()'>nPost</a></p>\n");
  }
  
  message += F("<iframe name='i' style='display:none' title='Tooltip' ></iframe>\n" // hack to keep the button press in the window
  //-----------------------------------------------------------------------------------------
  // end of simulation
  "</article>\n");



 //-----------------------------------------------------------------------------------------
  // Print error buffer
  message += F("<article>\n"

  //-----------------------------------------------------------------------------------------
  "<h2>Print errorbuffer</h2>\n" 
  "<p>Falls ein Reset-Fehler auftritt wird dieser in das error.log geschrieben "
  "<br>Dieses Wird hier angezeigt"
  "</p>");

  // open file for reading and check if it exists
  File file = LittleFS.open("/error.log", "r");
  if (!file) {
    Serial.println("Failed to open error.log nf for reading");
    message += "<br>File not found";
    
  } else {

    // read from file line by line
    // prepare loop
    // define locals
    char c;
    
    while (file.available()) { 
      c = file.read();

      if  (c=='\n'){
        message += "<br>";
      } else {
        message += c;
      }
    }
    file.close();
  }
  //-----------------------------------------------------------------------------------------
 
    addBottom(message);
  server.send(200, "text/html", message);
}

//*********************************************************************************
/* print value history*/
void handleListFiltered()
//*********************************************************************************
{
  // what string length do we need?
  //   listing shortterm 
  //   - per line 25
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

  //------------------------------------------------------------------
  // shortterm values
  //-------------------------------------------------------------------
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
  
  message += F("</pre></article><br>");

  addBottom(message);
  server.send(200, "text/html", message);
  
  Serial.print("2 - message length: "); 
  Serial.println(message.length());
}
//*********************************************************************************
/* print graph shortterm*/
void handleGraph()
//*********************************************************************************
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

//*********************************************************************************
void handleCss()
//*********************************************************************************
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
            );
  server.send(200, "text/css", message);
}

//*********************************************************************************
// Output: send data to browser as JSON
// after modification always check if JSON is still valid. Just call the JSON (json) in your webbrowser and check.
void handleJson()
{
//*********************************************************************************
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

//*********************************************************************************
// Output: a fetch API / JavaScript
// a function in the JavaScript uses fetch API to request a JSON file from the webserver and updates the values on the page if the object names and ID are the same
void handleJs()
//*********************************************************************************
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

//*********************************************************************************
void handleCommand()
//*********************************************************************************
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
    //reloadDone = 0;
    //Serial.println("reloadDone -> 0 -- Cmd = toggle");

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
    if (server.arg(0) == "0") // the value for that parameter
    {
      Serial.println(F("D232 toggle ahhh"));
      if (simVal_AHHH == 1) {simVal_AHHH = 0;} else {simVal_AHHH = 1;}
      digitalWrite(GPin_AHHH, simVal_AHHH);
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
      Serial.println(F("D232 toggle sendToClient"));
      if (digitalRead(builtin_led))
      { // toggle: if the pin was on - switch it of and vice versa
        digitalWrite(builtin_led, LOW);
        sendToClient = 1;  // enable sending to client in Wasserstand_V5.cpp
        Serial.println("sendToClient = 1");
      }
      else
      {
        digitalWrite(builtin_led, HIGH);
        if (isLiveSystem==0) {
          sendToClient = 0;  // disable sending to client in dev version ofWasserstand_V5.cpp
          Serial.println("sendToClient = 0");
        }
      }
    }
    if (server.arg(0) == "6") 
    {
      Serial.println(F("D232 toggle error generation"));
      if (simError==1)
      { // toggle error generation
        simError = 0;
        Serial.println("simError off");
      }
      else
      {
        // force error with sendPost to bplaced; response with negative code is forced
        if (debugLevelSwitches==1) {
          simError = 1;
          Serial.println("simError on");
        }
      }
    }
    if (server.arg(0) == "7") 
    {
      Serial.println(F("D232 toggle reboot generation"));
      if (simReboot==1)
      { // toggle error generation
        simReboot = 0;
        Serial.println("simReboot off");
      }
      else
      {
        // force error with sendPost to bplaced; response with negative code is forced
        if (debugLevelSwitches==1) {
          simReboot = 1;
          Serial.println("simReboot on");
        }
      }
    }
if (server.arg(0) == "8") 
    {
      Serial.println(F("D232 toggle useLiveMail"));
      if (useLiveMail==1)
      { // toggle usage of mail address
        useLiveMail = 0;
        Serial.println("useLiveMail off");
      }
      else
      {
        // use web.de
        if (debugLevelSwitches==1) {
          useLiveMail = 1;
          Serial.println("useLiveMail on");
        }
      }
    }
  }
  else if (server.argName(0) == "CMD" && server.arg(0) == "RESET") // Example how to reset the module. Just send ?CMD=RESET
  {
    Serial.println(F("D238 will reset"));
    ESP.restart();
  }

  // else if (server.argName(0) == "reloaded" && server.arg(0) == "true") 
  // {
  //   Serial.println(F("D238 will reload"));
  //   reloadDone = 1;
  // }

  // Serial.print("reloadDone : "); Serial.println(reloadDone);

// if (reloadDone==0) {
  if (false) {
    server.send(200, "text/html", "<html><body><script>localStorage.setItem('reloaded', 'true'); location.reload();</script></body></html>");
    // reloadDone = 1;
    Serial.println("200");
  } else {
    // reloadDone = 1;
    server.send(204, "text/plain", "No Content"); // this page doesn't send back content --> 204
    Serial.println("204");
  }
  
  setPegelforSimulation();

}

void handleData() {
// receives data from a remote board 
// and saves data to local variables
// it uses similar method like the command processing: 
// we receive parameters and store them in variables

  uint8_t counter = 0; // will count valid values
  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print(F(": "));
    Serial.println(server.arg(i));
    if (server.argName(i) == "board")
    {
      remoteBoardId = server.arg(0).toInt();
      counter++;
    }
    else if (server.argName(i) == "levelAct")
    {
      remoteLevelAct = server.arg(i).toInt();
      counter++;
    }
    else if (server.argName(i) == "debug_level_switches")
    {
      remoteDebugLevelSwitches = server.arg(i).toInt();
      counter++;
    }
    else if (server.argName(i) == "AHH")
    {
      remoteAHH = server.arg(i).toInt();
      counter++;
    }
    else if (server.argName(i) == "AH")
    {
      remoteAH = server.arg(i).toInt();
      counter++;
    }
    else if (server.argName(i) == "AL")
    {
      remoteAL = server.arg(i).toInt();
      counter++;
    }
  }
  //example for errorhandling
  if (counter >= 1)
  {
    remoteLastMessage = millis() / 1000; // store the timestamp 
    remoteMessagesSucessfull++; // increase successfull submits
  }
  server.send(200, "text/plain", "OK");
}

// *** Remote Page ***  r.htm
void handlePageR()
{
  Serial.println("handlePageR entered");
  
  String message;
  addTop(message);
  message += F("<article>\n"
               "<h2>Remote Module</h2>\n"
               "<p>This page shows values which were received from a remote module.<p>\n"
               "<table>\n"
               "<tr><th>variable</th><th>value</th></tr>\n"

               "<tr><td>remoteBoardId</td><td>");
  message += remoteBoardId;
  message += F("</td></tr>\n"

               "<tr><td>remoteLevelAct</td><td>");
  message += remoteLevelAct;
  message += F("</td></tr>\n"

               "<tr><td>remoteDebugLevelSwitches</td><td>");
  message += remoteDebugLevelSwitches;
  message += F("</td></tr>\n"

               "<tr><td>AHH</td><td>");
  message += remoteAHH;
  message += F("</td></tr>\n"

               "<tr><td>AH</td><td>");
  message += remoteAH;
  message += F("</td></tr>\n"

               "<tr><td>AL</td><td>");
  message += remoteAL;
  message += F("</td></tr>\n");

  if (remoteMessagesSucessfull > 0)
  {
    message += F("<tr><td>remoteMessagesSucessfull</td><td>");
    message += remoteMessagesSucessfull;
    message += F("</td></tr>\n"
                 "<tr><td>seconds since last message</td><td>");
    message += (millis() / 1000) - remoteLastMessage;
    message += F("</td></tr>\n");
  }
  else
  {
    message += F("<tr><td>no external data received so far</td><td>-</td>");
  }
  message += F("</table>\n"
               "<p>If this module (the 'server') receives data from another remote module (the 'client'), data will be displayed.</p>\n"
               "<p>The data for the remote module is not updated, therefore you will need to reload this page. You can modify the handleJson() and this page to add that data.</p>\n"
               "</article>\n"
              );
  addBottom(message);
  server.send(200, "text/html", message);
}


//}
/*
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
========== end of including server.cpp
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
*/
