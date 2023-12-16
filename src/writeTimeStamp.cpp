#include <Arduino.h>
#include <SD_MMC.h>
/**
 * Append a timestamp to a file
 * Just an example that shows how to continuously 
 * write values to a file on the SD card
*/
void writeTimeStamp(char path[])
{
  char timestamp[16];
  time_t now;
  
  // Open file on microSD card in append mode
  fs::FS &fs = SD_MMC;
  File file = fs.open(path, FILE_APPEND, true); // create if not existing

  if(file)
  {
    time(&now); // get epoche time stamp
    snprintf(timestamp, sizeof(timestamp), "%lu\n", now);
    file.write((uint8_t *)timestamp, strlen(timestamp));  // write timestamp to file as string
    file.close();
    Serial.printf("Timestamp: %lu appended to file: %s\n", now, path);
  }
  else
  {
    Serial.printf("Failed to open file in write mode");
  }
}
