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
////
//#include <Arduino.h>//
//#include <WiFi.h>//
//// for Send-Mail
//#include <ESP_Mail_Client.h>//
//#include <waterlevel_defines.h>//
//#include <timeserver.h>
//#include <waterlevel.h>//
//int val_AHH;
//int val_AH;
//int val_AL;
//int val_ALL;//
//// ESP8266WebServer server(80); // an instance for the webserver//////
///* =======================================*/
//void handleNotFound()
//{
//  /* =======================================*/
//  // Output a "404 not found" page. It includes the parameters which comes handy for test purposes.
//  Serial.println(F("D015 handleNotFound()"));
//  String message;
//  message += F("404 - File Not Found\n"
//               "URI: ");
//  message += server.uri();
//  message += F("\nMethod: ");
//  message += (server.method() == HTTP_GET) ? "GET" : "POST";
//  message += F("\nArguments: ");
//  message += server.args();
//  message += F("\n");
//  for (uint8_t i = 0; i < server.args(); i++)
//  {
//    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//  }
//  server.send(404, "text/plain", message);
//}//
///* =======================================*/
//void handle204()
///* =======================================*/
//{
//  server.send(204); // this page doesn't send back content
//}//
///* =======================================*/
///* add a header  to each page including refs for all other pages */
//void addTop(String &message)
///* =======================================*/
//{
//  message = F("<!DOCTYPE html>"
//              "<html lang='en'>"
//              "<head>"
//              "<title>Wasserstand-Messung</title>"
//              "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
//              "<meta name=\"viewport\" content=\"width=device-width\">"
//              "<link rel='stylesheet' type='text/css' href='/f.css'>"
//              "<script src='j.js'></script>"
//              "</head>");
//  message += F("<body>");
//  message += F("<header><h1>" TXT_BOARDNAME " - Board " TXT_BOARDID "</h1>");
//  message += F("<nav> <a href=\"/\">[Home]</a> <a href=\"filtered.htm\">[Value History]</a>" 
//               "<a href=\"graph.htm\">[Graph]</a> </nav></header>"
//               "<main>");
//}//
///* =======================================*/
///* The footer will display the uptime, the IP-address the version of the sketch and the compile date/time */
///* add code to calculate the uptime */
//void addBottom(String &message)
//{
//  /* =======================================*///
//  message += F("</main>"
//               "<footer><p>");
//  message += F("<p>Actual Date and Time: ");//
//  message += currentDate;
//  message += F(" -- ");
//  message += formattedTime;
//  message += F("<br>");//
//  if (seconds_since_startup > 604800)
//  {
//    message += F("<span id='week'>");
//    message += ((seconds_since_startup / 604800) % 52);
//    message += F("</span> weeks ");
//  }
//  if (seconds_since_startup > 86400)
//  {
//    message += F("<span id='day'>");
//    message += ((seconds_since_startup / 86400) % 7);
//    message += F("</span> days ");
//  }
//  if (seconds_since_startup > 3600)
//  {
//    message += F("<span id='hour'>");
//    message += ((seconds_since_startup / 3600) % 24);
//    message += F("</span> hours ");
//  }//
//  message += F("<span id='min'>");
//  message += ((seconds_since_startup / 60) % 60);
//  message += F("</span> minutes ");//
//  message += F("<span id='sec'>");
//  message += (seconds_since_startup % 60);
//  message += F("</span> seconds since startup | Version " VERSION " | IP: ");
//  message += WiFi.localIP().toString();
//  message += F(" | " __DATE__ " " __TIME__ "</p></footer></body></html>");
//  server.send(200, "text/html", message);
//}//
//// the html output
//// finally check your output if it is valid html: https://validator.w3.org
//// *** HOME ***  0.htm
///* =======================================*/
///* main page of this application:
// *   - display water level as raw data
// *   - diaplay actual warn or alarm satus
// */
//void handlePage()
///* =======================================*/
//{
//  String message;
//  addTop(message);//
//  message += F("<article>"
//               "<h2>Wasserstand Zehentner Teisendorf</h2>" // here you write your html code for your homepage. Let's give some examples...
//               "<p>Hier kann man den aktuellen Wasserstand in der Regenwasser-Zisterne  "
//               "von Georg Zehentner, Streiblweg 19, Teisendorf ablesen.<br> "
//               " Bei Überschreiten des Höchststand muss eine Pumpe aktiviert werden</p>"
//               "</article>");//
//  message += F("<article>"
//               "<h2>Rohdaten</h2><pre>");//
//  // input signals are low active
//  message += F("Level Alarm   high: <span id='val_AHH'>");
//  if (val_AHH == 0)
//  {
//    message += F("active");
//  }
//  else
//  {
//    message += F("--");
//  }
//  message += F("</span><br>");
//  message += F("Level Warning high: <span id='val_AH' >");
//  if (val_AH == 0)
//  {
//    message += F("active");
//  }
//  else
//  {
//    message += F("--");
//  }
//  message += F("</span><br>");
//  message += F("Level Warning low:  <span id='val_AL' >");
//  if (val_AL == 0)
//  {
//    message += F("active");
//  }
//  else
//  {
//    message += F("--");
//  }
//  message += F("</span><br>");
//  message += F("Level Alarm   low:  <span id='val_ALL'>");
//  if (val_ALL == 0)
//  {
//    message += F("active");
//  }
//  else
//  {
//    message += F("--");
//  }
//  message += F("</span><br>");
//  message += F("</pre></article>");//
//  message += F("<article>"
//               "<h2>Auswertung Wasserstand</h2>");
//  if (alarmState == 5)
//  {
//    message += F("<message_err> Achtung Hochwasser -- Pumpe einschalten <br>Wasserstand &gt ");
//    message += Level_AHH;
//    message += F("<br></message_err>");
//  }
//  else if (alarmState == 4)
//  {
//    message += F("<message_warn> Wasserstand ist zwischen ");
//    message += Level_AH;
//    message += F(" und ");
//    message += Level_AHH;
//    message += F("<br></message_warn>");
//  }
//  else if (alarmState == 3)
//  {
//    message += F("<message_ok> Wasserstand ist zwischen ");
//    message += Level_AL;
//    message += F(" und ");
//    message += Level_AH;
//    message += F("<br></message_ok>");
//  }
//  else if (alarmState == 2)
//  {
//    message += F("<message_ok> Wasserstand ist zwischen ");
//    message += Level_ALL;
//    message += F(" und ");
//    message += Level_AL;//
//    message += F("<br></message_ok>");
//  }
//  else if (alarmState == 1)
//  {
//    message += F("<message_ok>   Wasserstand &lt; ");
//    message += Level_ALL;
//    message += F("<br></message_ok>");
//  }//
//  // print when a new value arrieves
//  message += F("<br><br>  Zeit: ");
//  message += formattedTime;
//  message += F("   Wasserstand aktuell: ");
//  message += myValueFilteredAct;//
//  message += F("</article>");//
//  addBottom(message);
//  server.send(200, "text/html", message);
//}////
///* =======================================*/
///* print value history*/
//void handleListFiltered()
///* =======================================*/
//{
//  String message = "";
//  addTop(message);//
//  message += F("<article>"
//               "<h2>Wasserstand Zehentner Teisendorf</h2>" 
//               "<p>Index ");
//  message += filterCnt;             
//  message += "<br> </p>";//
//  // create header of table
//  message += "<pre>rdRingPtr  Time     Value ADC <br>";
//  
//  // read out ringbuffer and display values
//    for ( rdRingPtr = wrRingPtr+1; rdRingPtr != wrRingPtr; ){//
//      // align values 
//      if (rdRingPtr<10) {
//        //message += " &nbsp";
//      }
//      message += rdRingPtr;
//      message += "       ";
//      message += ringTime[rdRingPtr];
//      message += "  ";
//      message += ringValue[rdRingPtr];
//      message += "  ";
//      message += ringADC[rdRingPtr];
//      message += "<br>";//
//      if (rdRingPtr<iRingValueMax) {
//        rdRingPtr++;
//      } else {
//        rdRingPtr = 0;
//      }//
//    }
//    message += F("</pre></article><br>");//
//  addBottom(message);
//  server.send(200, "text/html", message);
//}
///* =======================================*/
///* print graph*/
//void handleGraph()
///* =======================================*/
//{
//  String message;
//  addTop(message);////
//  message += F("<article>"
//               "<h2>Wasserstand Zehentner Teisendorf</h2>" 
//               "<p>Line Graph<br> </p> "); //
//  message += "<br>filterCnt= ";
//  message += filterCnt;//
//  // print when a new value arrieves
//  message += F("<br><br>  Zeit: ");
//  message += formattedTime;
//  message += F("   Wasserstand aktuell: ");
//  message += myValueFilteredAct;
//  message += "</article>";//
//  message += "<script"
//               " src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js\">"
//               "</script>";
//  
//  message += "<canvas id=\"myChart\" style=\"width:100%;max-width:700px\"></canvas>";
//  
//  message += "<script>";
//  message += graphXValues; 
//  message += graphYValues; 
//  message += graphYlevelWarn;
//  message += graphYlevelErro;
//  
//  message +=  "new Chart(\"myChart\", {";
//  message +=  " type: \"line\",";
//  message +=  " data: {";
//  message +=  "   labels: xValues,";
//  message +=  "   datasets: [{";
//  message +=  "     fill: false,";
//  message +=  "     lineTension: 0,";
//  message +=  "     backgroundColor: \"rgba(0,0,255,1.0)\",";
//  message +=  "     borderColor: \"rgba(0,0,255,0.5)\",";
//  message +=  "     label: \"Wasserstand [mm]\",";
//  message +=  "     data: yValues";
//  message +=  "     },{";
//  message +=  "     fill: false,";
//  message +=  "     lineTension: 0,";
//  message +=  "     backgroundColor: \"rgba(0,255,0,0)\",";
//  message +=  "     borderColor: \"rgba(0,255,0,0.3)\",";
//  message +=  "     label: \"Warnschwelle: ";
//  message +=  Level_AH;
//  message +=  "0 mm\",";
//  message +=  "     data: yLevelWarn";
//  message +=  "     },{";
//  message +=  "     fill: false,";
//  message +=  "     lineTension: 0,";
//  message +=  "     backgroundColor: \"rgba(255,0,0,0)\",";
//  message +=  "     borderColor: \"rgba(255,0,0,0.3)\",";
//  message +=  "     label: \"Alarmschwelle: ";
//  message +=  Level_AHH;
//  message +=  "0 mm\",";
//  message +=  "     data: yLevelErro";
//  message +=  "   }]";
//  message +=  " },";
//  message +=  " options: {";
//  message +=  "   title: {";
//  message +=  "   display: false,";
//  message +=  "   text: \"Wasserstand in mm\"";
//  message +=  "   },";
//  message +=  "   legend: {display: true, text: \"Wasserstand \"},";
//  message +=  "   scales: {";
//  message +=  "     yAxes: [{ticks: {min: 1000, max:2000}}]";
//  message +=  "   }";
//  message +=  " }";
//  message +=  " });";//
//  message +=  "</script>";//
//  // message +=  "</body>";
//  //message +=  "</html>";
//  
//  addBottom(message);
//  server.send(200, "text/html", message);
//}//
///* =======================================*/
//void handleCss()
///* =======================================*/
//{
//  // output of stylesheet
//  // this is a straight forward example how to generat a static page from program memory
//  String message;
//  message = F("*{font-family:sans-serif}"
//              "body{margin:10px}"
//              "h1, h2{color:white;background:" CSS_MAINCOLOR ";text-align:center}"
//              "h1{font-size:1.2em;margin:1px;padding:5px}"
//              "h2{font-size:1.0em}"
//              "h3{font-size:0.9em}"
//              "a{text-decoration:none;color:dimgray;text-align:center}"
//              "main{text-align:center}"
//              "article{vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:#E5E5E5;width:20em;text-align:left}" // if you don't like the floating effect (vor portrait mode on smartphones!) - remove display:inline-block
//              "article h2{margin-top:0;padding-bottom:1px}"
//              "section {margin-bottom:0.2em;clear:both;}"
//              "table{border-collapse:separate;border-spacing:0 0.2em}"
//              "th, td{background-color:#C0C0C0}"
//              "button{margin-top:0.3em}"
//              "footer p{font-size:0.8em;color:dimgray;background:silver;text-align:center;margin-bottom:5px}"
//              "nav{background-color:silver;margin:1px;padding:5px;font-size:0.8em}"
//              "nav a{color:dimgrey;padding:10px;text-decoration:none}"
//              "nav a:hover{text-decoration:underline}"
//              "nav p{margin:0px;padding:0px}"
//              ".on, .off{margin-top:0;margin-bottom:0.2em;margin-left:4em;font-size:1.4em;background-color:#C0C0C0;border-style:solid;border-radius:10px;border-style:outset;width:5em;height:1.5em;text-decoration:none;text-align:center}"
//              ".on{border-color:green}"
//              ".off{border-color:red}"
//              "message_ok  {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:green ;width:19em;text-align:center}"
//              "message_warn{color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:orange;width:19em;text-align:center}"
//              "message_err {color:white;vertical-align:top;display:inline-block;margin:0.2em;padding:0.1em;border-style:solid;border-color:#C0C0C0;background-color:red   ;width:19em;text-align:center}");
//  server.send(200, "text/css", message);
//}//
///* =======================================*/
//// Output: send data to browser as JSON
//// after modification always check if JSON is still valid. Just call the JSON (json) in your webbrowser and check.
//void handleJson()
//{
//  /* =======================================*/
//  //  Serial.println(F("D268 requested json"));
//  String message = "";
//  message = (F("{\"ss\":")); // Start of JSON and the first object "ss":
//  message += millis() / 1000;
//  message += (F(",\"val_AHH\":"));
//  message += digitalRead(GPin_AHH);
//  message += (F(",\"val_AH\":"));
//  message += digitalRead(GPin_AH);
//  message += (F(",\"val_AL\":"));
//  message += digitalRead(GPin_AL);
//  message += (F(",\"val_ALL\":"));
//  message += digitalRead(GPin_ALL);
//  message += (F("}"));                           // End of JSON
//  server.send(200, "application/json", message); // set MIME type https://www.ietf.org/rfc/rfc4627.txt
//}//
///* =======================================*/
//// Output: a fetch API / JavaScript
//// a function in the JavaScript uses fetch API to request a JSON file from the webserver and updates the values on the page if the object names and ID are the same
//void handleJs()
//{
//  /* =======================================*/
//  String message;
//  message += F("const url ='json';\n"
//               "function renew(){\n"
//               " document.getElementById('sec').style.color = 'blue'\n" // if the timer starts the request, the second gets blue
//               " fetch(url)\n"                                          // Call the fetch function passing the url of the API as a parameter
//               " .then(response => {return response.json();})\n"
//               " .then(jo => {\n"
//               "   document.getElementById('sec').innerHTML = Math.floor(jo['ss'] % 60);\n" // example how to change a value in the HTML page
//               "   for (var i in jo)\n"
//               "    {if (document.getElementById(i)) document.getElementById(i).innerHTML = jo[i];}\n" // as long as the JSON name fits to the HTML id, the value will be replaced
//               // add other fields here (e.g. the delivered JSON name doesn't fit to the html id
//               // finally, update the runtime
//               "   if (jo['ss'] > 60) { document.getElementById('min').innerHTML = Math.floor(jo['ss'] / 60 % 60);}\n"
//               "   if (jo['ss'] > 3600) {document.getElementById('hour').innerHTML = Math.floor(jo['ss'] / 3600 % 24);}\n"
//               "   if (jo['ss'] > 86400) {document.getElementById('day').innerHTML = Math.floor(jo['ss'] / 86400 % 7);}\n"
//               "   if (jo['ss'] > 604800) {document.getElementById('week').innerHTML = Math.floor(jo['ss'] / 604800 % 52);}\n"
//               "   document.getElementById('sec').style.color = 'dimgray';\n" // if everything was ok, the second will be grey again.
//               " })\n"
//               " .catch(function() {\n" // this is where you run code if the server returns any errors
//               "  document.getElementById('sec').style.color = 'red';\n"
//               " });\n"
//               "}\n"
//               "document.addEventListener('DOMContentLoaded', renew, setInterval(renew, ");
//  message += ajaxIntervall * 1000;
//  message += F("));");//
//  server.send(200, "text/javascript", message);
//}//
///*
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
//========== end of including server.cpp
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*==*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=
//*///