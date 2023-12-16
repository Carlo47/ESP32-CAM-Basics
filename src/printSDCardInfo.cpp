#include <Arduino.h>
#include <SD_MMC.h>

/**
 * Use a raw string literal to print a formatted
 * string of SD card details
*/
void printSDCardInfo()
{
  const char *knownCardTypes[] = {"NONE", "MMC", "SDSC", "SDHC", "UNKNOWN"};
  sdcard_type_t cardType = SD_MMC.cardType();
  uint64_t cardSize  = SD_MMC.cardSize() >> 20; // divide by 2^20 = 1'048'576 to get size in MB
  uint64_t cardTotal = SD_MMC.totalBytes() >> 20;
  uint64_t cardUsed  = SD_MMC.usedBytes() >>  20;
  uint64_t cardFree  = cardTotal - cardUsed; 
  Serial.printf(R"(
SDCard Info
-----------
  type : %s
  size : %6llu MB
  total: %6llu MB
  used : %6llu MB
  free : %6llu MB
)", knownCardTypes[cardType], cardSize, cardTotal, cardUsed, cardFree); 
}