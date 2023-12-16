#include <Arduino.h>
#include <WIFI.h>

// WiFi credentials 
const char ssid[]      = "your ssid";
const char password[]  = "your password";
const char HOST_NAME[] = "ESP-CAM_BASIC";

/**
 * Establish the WiFi connection with router 
 * and set a hostname for the ESP32
*/
void initWiFi()
{
  Serial.println("Connecting to WiFi");
  WiFi.setHostname(HOST_NAME);
  WiFi.begin(ssid, password);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("...Connecting to WiFi");
    delay(1000);
  }
  Serial.println("Connected");
  log_i("===> done");
}