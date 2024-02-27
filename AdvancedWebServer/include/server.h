
/*
Include file for web-server things

*/
#ifndef SERVER_H
#define SERVER_H

// #include <WiFi.h>
// #include <WiFiClient.h>
//#include <WebServer.h>
// #include <ESPmDNS.h>


//extern WebServer server; // declare an instance for the webserver

void handleRoot();

// /* =======================================*/
// // Output a "404 not found" page. It includes the parameters which comes handy for test purposes.
// void handleNotFound();
void handleNotFound();

/* =======================================*/
// this page doesn't send back content
void handle204();

/* =======================================*/
/* main page of this application:
 *   - display water level as raw data
 *   - diaplay actual warn or alarm satus
 */
void handlePage();

/* =======================================*/
/* print filtered values*/
void handleListFiltered();

/* =======================================*/
/* print graph*/
void handleGraph();
/* =======================================*/

void drawGraph();

/* =======================================*/
// output of stylesheet
void handleCss();

/* =======================================*/
// Output: send data to browser as JSON
// after modification always check if JSON is still valid. Just call the JSON (json) in your webbrowser and check.
void handleJson();

/* =======================================*/
// Output: a fetch API / JavaScript
// a function in the JavaScript uses fetch API to request a JSON file from the webserver and updates the values on the page if the object names and ID are the same
void handleJs();

#endif