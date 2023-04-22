/**
 * Program      esp32CamBasics.cpp
 * 
 * Author       2023-04-22 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Tests the basic functions of the ESP32-CAM board:
 *                - Initialize WiFi connection and show connection details
 *                - Get time information from NTP server and set RTC of the ESP32 with time zone info
 *                - Initialize SD card and show card info
 *                - Blink LED_BUILTIN at GPIO_33
 *                - Flash FLASH_LIGHT at GPIO_4 with set brightness
 *                - Open a file and write periodically a timestamp into it 
 *                - Take a phote at a button click and save it to a file on the SD card
 *
 * Board        ESP32-CAM
 *
 * Wiring                                 ___________
 *                               .-------|  SD card  |-------.
 *                   FTDI 5V <---o 5V    |  .-----.  |   3V3 o
 *                     .---------o GND   |  |( o )|  |  IO16 o
 *                     |   _T_   o IO12  |  '-cam-'  |   IO0 o---/ to GND for uploading
 *                     '---o o---o IO13  |    ¦ ¦    |   GND o
 *                       Button  o IO15  |____¦ ¦____|   Vcc o 
 *                               o IO14       ¦ ¦        UOR o---> TX  of FTDI
 *                               o IO2        ¦ ¦        UOT o---> RX  of FTDI          
 *                               o IO4        ¦ ¦        GND o---> GND of FTDI
 *                               |        !!!!!!!!!!!        |
 *                               |                           |
 *                               |                 FLASH [x] | 
 *                               |                           |
 *                               |         ESP32-CAM         |
 *                               '---------------------------'
 *
 * Remarks      ESP32-CAM must be programmed with an FTDI-Adapter (UART->USB) as follows:
 *                - Connect IO0 to ground
 *                - Compile code, on SUCCESS push reset button on the backside of the ESP32-CAM
 *                - Upload code
 *                - Disconnect IO0 from ground
 *                - Push reset button to run code
 *  
 * References    https://randomnerdtutorials.com/projects-esp32-cam/ 
 *               https://github.com/bitluni/ESP32CamTimeLapse 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <esp_camera.h>

const int  PIN_BUTTON        = 13;
const int  LED_BUILTIN       = 33;
const int  FLASH_LIGHT       = 4;
const bool FLASH_DISABLED    = true;
const int  PWM_FLASH_FREQ    = 5000;  // PWM settings
const int  PWM_FLASH_CHANNEL = 15;    // camera uses timer1
const int  PWM_FLASH_RESOLUTION_BITS = 10;     // resolution (8 = 0..255, 10 = 0..1023)
const int  FLASH_BRIGHTNESS  = 300; 
const char NTP_SERVER_POOL[] = "ch.pool.ntp.org";
const char TIME_ZONE[]       = "MEZ-1MESZ-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; 
const int  TIME_FORMAT       = 5;  // 0..6 see function printDateTime()
const char HOST_NAME[]       = "ESP-CAM_BASIC";
const char SD_DATA_DIR[]     = "/DATA/";  //store data file in a subdirectory
const char SD_IMG_DIR[]      = "/DCIM/";  //store photos in a subdirectory

// WiFi credentials 
const char ssid[]     = "your ssid";
const char password[] = "your pwd";

const uint32_t secondsToSynchronize = 5; // output date and time every full 5 seconds
char dataPath[48];  // holds data filename
char imgPath[48];   // holds img filename

// Pin definition for CAMERA_MODEL_AI_THINKER
// Change pin definition if you're using another ESP32 camera module
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


/**
 * Returns true, as soon as msWait milliseconds have passed.
 * Supply a reference to an unsigned long variable to hold 
 * the previous milliseconds.
*/
bool waitIsOver(uint32_t &msPrevious, uint32_t msWait)
{
  return (millis() - msPrevious > msWait) ? (msPrevious = millis(), true) : false;
}

/**
 * Debounce the button and return true when
 * a click event is detected
*/
bool buttonClicked(uint8_t pinButton)
{
  static uint8_t button_history = 0;
  bool clicked = false;    

  button_history = button_history << 1;
  button_history |= (uint8_t)digitalRead(pinButton);

  if ((button_history & 0b11000011) == 0b11000000)
  { 
    clicked = true;
    button_history = 0;
  }
  return clicked;
}


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


/**
 * Use a raw string literal to print a formatted
 * string of WiFi connection details
*/
void printConnectionDetails()
{
  Serial.printf(R"(
Connection Details:
------------------
  SSID       : %s
  Hostname   : %s
  IP-Address : %s
  MAC-Address: %s
  RSSI       : %d (received signal strength indicator)
  )", WiFi.SSID().c_str(),
      //WiFi.hostname().c_str(),  // ESP8266
      WiFi.getHostname(),    // ESP32 
      WiFi.localIP().toString().c_str(),
      WiFi.macAddress().c_str(),
      WiFi.RSSI());
  Serial.printf("\n");
}


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
  SDCard type: %s
    size : %6llu MB
    total: %6llu MB
    used : %6llu MB
    free : %6llu MB
)", knownCardTypes[cardType], cardSize, cardTotal, cardUsed, cardFree); 
}


/**
 * Establish the WiFi connection with router
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


/**
 * Initialize the ESP32 RTC with local time
 * and close the no longer needed WiFi connection
*/
void initRTC(const char timeZone[])
{
  tm rtcTime;
  configTzTime(timeZone, NTP_SERVER_POOL);
  while(! getLocalTime(&rtcTime))
  {
    Serial.println("...Failed to obtain time");
  }
  log_i("Got time from NTP Server");
  WiFi.disconnect(true); //rtc is set, wifi connection no longer needed
  log_i("==> done");
}


void initSDCard()
{
  while(! SD_MMC.begin("/sdcard", FLASH_DISABLED)) delay(5000);
  while(SD_MMC.cardType() == CARD_NONE) { Serial.println("...no SD Card attached"); delay(5000); }
  log_i("==> done");
}


void setFlashBrightness(int brightness)
{
  ledcWrite(PWM_FLASH_CHANNEL, brightness);   // change LED brightness (0..255, resp. 0..1023)
}


void initFlashLight() 
{
    ledcSetup(PWM_FLASH_CHANNEL, PWM_FLASH_FREQ, PWM_FLASH_RESOLUTION_BITS);
    ledcAttachPin(FLASH_LIGHT, PWM_FLASH_CHANNEL);
    log_i("==> done");
}


void adjustSensorParams()
{
  sensor_t *s = esp_camera_sensor_get();
 
  s->set_brightness(s, 0);     // BRIGHTNESS (-2 to 2)
  s->set_contrast(s, 0);       // CONTRAST (-2 to 2)
  s->set_saturation(s, 0);     // SATURATION (-2 to 2)
  s->set_special_effect(s, 0); // SPECIAL EFFECTS  0 - No Effect, 
                               //                  1 - Negative, 
                               //                  2 - Grayscale, 
                               //                  3 - Red Tint, 
                               //                  4 - Green Tint, 
                               //                  5 - Blue Tint, 
                               //                  6 - Sepia
  s->set_whitebal(s, 1);       // WHITE BALANCE (0 = Disable , 1 = Enable)
  s->set_awb_gain(s, 1);       // AWB GAIN (0 = Disable , 1 = Enable)
  s->set_wb_mode(s, 0);        // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // EXPOSURE CONTROLS (0 = Disable , 1 = Enable) 
  s->set_aec2(s, 0);           // AEC2 (0 = Disable , 1 = Enable)
  s->set_ae_level(s, 0);       // AE LEVELS (-2 to 2)
  s->set_aec_value(s, 300);    // AEC VALUES (0 to 1200)
  s->set_gain_ctrl(s, 1);      // GAIN CONTROLS (0 = Disable , 1 = Enable)
  s->set_agc_gain(s, 0);       // AGC GAIN (0 to 30)
  s->set_gainceiling(s, (gainceiling_t)0); // GAIN CEILING (0 to 6)
  s->set_bpc(s, 0);            // BPC (0 = Disable , 1 = Enable)
  s->set_wpc(s, 1);            // WPC (0 = Disable , 1 = Enable)
  s->set_raw_gma(s, 1);        // RAW GMA (0 = Disable , 1 = Enable)
  s->set_lenc(s, 1);           // LENC (0 = Disable , 1 = Enable)
  s->set_hmirror(s, 0);        // HORIZ MIRROR (0 = Disable , 1 = Enable)
  s->set_vflip(s, 0);          // VERT FLIP (0 = Disable , 1 = Enable)
  s->set_dcw(s, 1);            // DCW (0 = Disable , 1 = Enable)
  s->set_colorbar(s, 0);       // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
}


void initCamera()
{
  // Stores the camera configuration parameters
  camera_config_t camConfig = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,   // YUV422, GRAYSCALE, RGB565, JPEG
    .grab_mode = CAMERA_GRAB_LATEST,  // CAMERA_GRAB_WHEN_EMPTY
  };

  // Select lower framesize if the camera doesn't support PSRAM
  if(psramFound())
  {
    camConfig.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    camConfig.jpeg_quality = 10; //0-63 lower number means higher quality
    camConfig.fb_count = 1;
  } else 
  {
    camConfig.frame_size = FRAMESIZE_SVGA;
    camConfig.jpeg_quality = 12;
    camConfig.fb_count = 1;
  }
  
  // Initialize the Camera
  while (esp_err_t err = esp_camera_init(&camConfig) != ESP_OK) 
  {
    log_i("...failed with error 0x%x", err);
    delay(1000);
  }
  log_i("==> done");
}


/**
 * Synchronize the output to full seconds
*/
void synchronize(uint32_t toSeconds)
{
  tm rtcTime;
  bool synchronized = false;

  while (! synchronized) 
  {
    getLocalTime(&rtcTime);
    if (rtcTime.tm_sec % toSeconds == 0) 
    {  
      synchronized = true;
      printDateTime(TIME_FORMAT);
    }
  } 
}

/**
 * Compose a file name in the form /DIR/prefixYYYYMMDD_hhmmss.fileExt
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
}


/**
 * Append a timestamp to a file
 * Just an example that shows how to continuously 
 * write values to a file on the SD card
*/
void writeTimeStamp(char path[])
{
  char timestamp[16];
  time_t now;
  
  // Save timestamp to microSD card
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


void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FLASH_LIGHT,  OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  initWiFi();         printConnectionDetails();
  initRTC(TIME_ZONE); printDateTime(2);
  initSDCard();       printSDCardInfo();
  initFlashLight();   setFlashBrightness(5);
  initCamera();       adjustSensorParams();
  composeFilename(dataPath, SD_DATA_DIR, "data", ".txt");
  log_i("==> data file name composed: %s", dataPath);
}


void loop() 
{
  const int period = 1000;
  const int pulsewidth = 50;
  const int flash_period = 3000;
  const int flash_pulsewidth = 2;
  static uint32_t msPrevious = millis();

  // Blink the builtin led every second for 50 ms
  digitalWrite(LED_BUILTIN, (millis()) % period < pulsewidth ? LOW : HIGH);

  // Flash the white led every 3 seconds for 2 ms
  (millis() % flash_period < flash_pulsewidth) ? ledcWrite(PWM_FLASH_CHANNEL, FLASH_BRIGHTNESS) : 
                                                 ledcWrite(PWM_FLASH_CHANNEL, 0);

  // Print the current date and time
  // and append a timestamp to a file
  if (waitIsOver(msPrevious, secondsToSynchronize * 1000)) 
    {
      printDateTime(TIME_FORMAT);
      writeTimeStamp(dataPath);
    }
  
  // Take a photo at the click of the button
  if (buttonClicked(PIN_BUTTON)) 
    {
      takePhotoAndSave();
      log_i("==> Photo taken");
    }
}