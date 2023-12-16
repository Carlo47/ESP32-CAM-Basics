#include <Arduino.h>
#include <WiFi.h>

const char NTP_SERVER_POOL[] = "ch.pool.ntp.org";

/**
 * Initialize the ESP32 RTC with local time
 * and close the no longer needed WiFi connection
 * when disconnect is passed as true, default is false
*/
void initRTC(const char timeZone[], bool disconnect = false)
{
  tm rtcTime;
  configTzTime(timeZone, NTP_SERVER_POOL);
  while(! getLocalTime(&rtcTime))
  {
    Serial.println("...Failed to obtain time");
  }
  log_i("Got time from NTP Server");
  if (disconnect) WiFi.disconnect(true); //rtc is set, wifi connection no longer needed
  log_i("==> done");
}