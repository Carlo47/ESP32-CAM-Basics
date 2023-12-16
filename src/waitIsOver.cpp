#include <Arduino.h>
/**
 * Returns true, as soon as msWait milliseconds have passed.
 * Supply a reference to an unsigned long variable to hold 
 * the previous milliseconds.
*/
bool waitIsOver(uint32_t &msPrevious, uint32_t msWait)
{
  return (millis() - msPrevious > msWait) ? (msPrevious = millis(), true) : false;
}