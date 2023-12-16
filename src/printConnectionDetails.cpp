#include <Arduino.h>
#include <WiFi.h>

/**
 * Use a raw string literal to print a formatted
 * string of WiFi connection details
*/
void printConnectionDetails()
{
  Serial.printf(R"(
Connection Details
------------------
  SSID       : %s
  Hostname   : %s
  IP-Address : %s
  MAC-Address: %s
  RSSI       : %d (received signal strength indicator)
  )", WiFi.SSID().c_str(),
      //WiFi.hostname().c_str(),  // ESP8266
      WiFi.getHostname(),    // ESP32 
      WiFi.localIP().toString().c_str(),
      WiFi.macAddress().c_str(),
      WiFi.RSSI());
}