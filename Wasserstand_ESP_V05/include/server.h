
/*
Include file for web-server things

*/
#ifndef SERVER_H
#define SERVER_H

#include <waterlevel_defines.h>

#if BOARDTYPE == ESP8266
    #include <ESP8266WebServer.h> // for the webserver
    extern ESP8266WebServer server; // declare an instance for the webserver
#endif

extern int debugLevelSwitches;

extern int simVal_AHH;
extern int simVal_AH ;
extern int simVal_AL ;
extern int simVal_ALL;

//extern WebServer server; // declare an instance for the webserver
extern int sendToClient; // enable sending to client

// **************************************************************************************************
  void handleRoot();
// **************************************************************************************************
  
// **************************************************************************************************
// Output a "404 not found" page. It includes the parameters which comes handy for test purposes.
void handleNotFound();
// **************************************************************************************************

// **************************************************************************************************
// this page doesn't send back content
void handle204();
// **************************************************************************************************

// **************************************************************************************************
/* main page of this application:
 *   - display water level as raw data
 *   - diaplay actual warn or alarm satus
 */
void handlePage();
// **************************************************************************************************

// **************************************************************************************************
/* print filtered values*/
void handleListFiltered();
// **************************************************************************************************

// **************************************************************************************************
/* print graph for short term values*/
void handleGraph();
// **************************************************************************************************
// **************************************************************************************************
/* print graph for short term values with print on change using google chart*/
void handleGraph_POC();
// **************************************************************************************************

// **************************************************************************************************
/* print graph for longterm values*/
void handleLongtermGraph();
// **************************************************************************************************

// **************************************************************************************************
/* print both graph longterm and shortterm*/
void handleBothGraph();
// **************************************************************************************************

#if BOARDTYPE == ESP32

/* =======================================*/
void drawGraph();
/* =======================================*/
#endif


// **************************************************************************************************
// output of stylesheet
void handleCss();
// **************************************************************************************************

// **************************************************************************************************
// Output: send data to browser as JSON
// after modification always check if JSON is still valid. Just call the JSON (json) in your webbrowser and check.
void handleJson();
// **************************************************************************************************

// **************************************************************************************************
// Output: a fetch API / JavaScript
// a function in the JavaScript uses fetch API to request a JSON file from the webserver and updates the values on the page if the object names and ID are the same
void handleJs();
// **************************************************************************************************

// only first steps
// void handleSliderJs();
// void handleSlider();

// **************************************************************************************************
void handleCommand();
// **************************************************************************************************

// **************************************************************************************************/
// receives data from a remote board 
// and saves data to local variables
// it uses similar method like the command processing: 
// we receive parameters and store them in variables
// **************************************************************************************************/
void handleData();
// **************************************************************************************************/

// **************************************************************************************************/
// display data received from remote (handleData)
void handlePageR();
// **************************************************************************************************/


#endif