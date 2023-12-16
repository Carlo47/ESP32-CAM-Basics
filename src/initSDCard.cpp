#include <Arduino.h>
#include <SD_MMC.h>

const bool FLASH_DISABLED = true;

/**
 * Initialize the SD card
*/
void initSDCard()
{
  while(! SD_MMC.begin("/sdcard", FLASH_DISABLED)) delay(5000);
  while(SD_MMC.cardType() == CARD_NONE) { Serial.println("...no SD Card attached"); delay(5000); }
  log_i("==> done");
}