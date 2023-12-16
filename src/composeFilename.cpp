#include <Arduino.h>

/**
 * Compose a file name in the form /DIR/prefixYYYYMMDD_hhmmss.fileExt
 * an write it to the supplied buffer
*/
void composeFilename(char buf[], const char dir[], const char prefix[], const char fileExt[])
{
  tm timeinfo;
  char timestr[20];
  if(! getLocalTime(&timeinfo))
  {
    log_i("Failed to obtain time");
  }
  else
  {
    strftime(timestr, sizeof(timestr), "%Y%m%d_%H%M%S", &timeinfo); // ":" not allowed in filename
    sprintf(buf, "%s%s%s%s", dir, prefix, timestr, fileExt);
  }
  log_i("==> done: %s", buf);
}