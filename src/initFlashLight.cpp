#include <Arduino.h>

const int  PWM_FLASH_FREQ    = 5000;  // PWM settings
const int  PWM_FLASH_CHANNEL = 15;    // camera uses timer1
const int  PWM_FLASH_RESOLUTION_BITS = 10;     // resolution (8 = 0..255, 10 = 0..1023)
const int  FLASH_BRIGHTNESS  = 300;

/**
 * Initialize the flash led
*/
void initFlashLight(int pinFlashLed) 
{
  pinMode(pinFlashLed,  OUTPUT);
  ledcSetup(PWM_FLASH_CHANNEL, PWM_FLASH_FREQ, PWM_FLASH_RESOLUTION_BITS);
  ledcAttachPin(pinFlashLed, PWM_FLASH_CHANNEL);
  log_i("==> done");
}

/**
 * Set the flash led brightness 0..255, resp. 0..1023
 * according to the set pwm resolution (8 or 10 bits)
*/
void setFlashBrightness(int brightness)
{
  ledcWrite(PWM_FLASH_CHANNEL, brightness);   // change LED brightness (0..255, resp. 0..1023)
}

/**
 * Flash the white led every msFlashPeriod milliseconds
 * with a pulse duration of msFlashPulsewidth milliseconds
*/
void flashLed(int msFlashPeriod, int msFlashPulsewidth)
{
  (millis() % msFlashPeriod < msFlashPulsewidth) ? ledcWrite(PWM_FLASH_CHANNEL, FLASH_BRIGHTNESS) : 
                                                   ledcWrite(PWM_FLASH_CHANNEL, 0);
}
