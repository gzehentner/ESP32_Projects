

#include <Arduino.h>
#include <WiFi.h>

#include <myWiFiLib.h>
#include <waterlevel_defines.h>

const char *ssid = STASSID;
const char *password = STAPSK;

void connectWifi() {
    char myhostname[8] = {"esp"};
    strcat(myhostname, TXT_BOARDID);
    WiFi.hostname(myhostname);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
void restartWiFi()
{

    Serial.println("Restarting WiFi...");

    // Disconnect and turn off WiFi
    WiFi.disconnect(true); // true = disable WiFi at the chip level
    WiFi.mode(WIFI_OFF);
    delay(1000); // Give it some time to fully shutdown

    // Turn WiFi back on and reconnect
    WiFi.mode(WIFI_STA);        // Set WiFi to station mode
    WiFi.begin(ssid, password); // Replace SSID and PASSWORD with your credentials

    // Wait for connection with timeout
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20)
    {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi reconnected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nFailed to reconnect WiFi!");
    }
}