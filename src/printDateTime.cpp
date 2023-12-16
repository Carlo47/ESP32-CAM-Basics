#include <Arduino.h>
/**
 * Print date and time in various formats
*/
void printDateTime(int format)
{
  tm   rtcTime;
  char buf[40];
  int  bufSize = sizeof(buf);

  getLocalTime(&rtcTime);
  switch (format)
  {
    case 0:  
      strftime(buf, bufSize, "%T",                &rtcTime); // hh:mm:ss
    break;
    case 1:  
      strftime(buf, bufSize, "%F",                &rtcTime); // YYYY-MM-DD
    break;
    case 2:  
      strftime(buf, bufSize, "%B %d %Y %T (%A)",  &rtcTime); // January 15 2019 16:33:20 (Tuesday)
    break;
    case 3:  
      strftime(buf, bufSize, "%F %T",             &rtcTime); // 2019-01-15 16:33:20
    break;
    case 4:  
      strftime(buf, bufSize, "%c",                &rtcTime); // Tue Jan 15 16:33:20 2019
    break;
    case 5:  
      strftime(buf, bufSize, "%F %T %W/%w %Z %z", &rtcTime); // 2019-01-15 16:51:18 02/2 MEZ +0100
    break;
    default: 
      strftime(buf, bufSize, "%D %r",             &rtcTime ); // 08/23/01 02:55:02 pm
    break;
  }
  Serial.printf("%s\n", buf);
}
