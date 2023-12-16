#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_camera.h>

extern void composeFilename(char buf[], const char dir[], const char prefix[], const char fileExt[]);

const char SD_IMG_DIR[] = "/DCIM/";  //store photos in a subdirectory
char imgPath[48];   // holds img filename after calling composeFilename()

/**
 * Take a photo and save it to a new file on the SD card
*/
void takePhotoAndSave()
{
  camera_fb_t *fb = esp_camera_fb_get();
  
  if(! fb) 
  {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  composeFilename(imgPath, SD_IMG_DIR, "img", ".jpg");
  
  // Save picture to microSD card
  fs::FS &fs = SD_MMC; 
  File file = fs.open(imgPath, FILE_WRITE, true); // create the file if it doesn't exist
  if(file)
  {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.printf("Saved: %s\n", imgPath);
  }
  else
  {
    Serial.printf("Failed to open file in write mode");
  } 
  esp_camera_fb_return(fb); 
}
