/**
 * Program      esp32CamBasics.cpp
 * 
 * Author       2023-04-22 Charles Geiser (https://www.dodeka.ch)
 *              2023-12-16 New code structure. The functions are now in their own 
 *                         source files. They are declared as external in the main 
 *                         program. This now only contains a few variable definitions 
 *                         and the setup() and loop() functions.
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
 *              If you use the ESP32-CAM with the plug-in USB motherboard, programming is much
 *              easier, but the IO pin for the trigger button is no longer easily accessible.
 *  
 * References    https://randomnerdtutorials.com/projects-esp32-cam/ 
 *               https://github.com/bitluni/ESP32CamTimeLapse 
 */

#include <Arduino.h>

// The following functions are defined in their own source files
extern void adjustSensorParams();
extern bool buttonClicked(uint8_t pinButton);
extern void composeFilename(char buf[], const char dir[], const char prefix[], const char fileExt[]);
extern void flashLed(int msFlashPeriod, int msFlashPulsewidth);
extern void initCamera();
extern void initFlashLight(int pinFlashLed);
extern void initWiFi();
extern void initSDCard();
extern void initRTC(const char timeZone[], bool disconnect = false);
extern void printConnectionDetails();
extern void printDateTime(int format);
extern void printSDCardInfo();
extern void printSystemInfo();
extern void setFlashBrightness(int brightness);
extern void takePhotoAndSave();
extern bool waitIsOver(uint32_t &msPrevious, uint32_t msWait);
extern void writeTimeStamp(char path[]);

const int  PIN_FLASH_LIGHT   = 4;     // Pin of flash led
const int  PIN_BUTTON        = 13;
const int  LED_BUILTIN       = 33;
const bool DISCONNECT_WIFI   = false;

const char TIME_ZONE[]       = "MEZ-1MESZ-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; 
const int  TIME_FORMAT       = 5;  // 0..6 see function printDateTime()
const char SD_DATA_DIR[]     = "/DATA/";  //store data file in a subdirectory

char dataPath[48];              // holds data filename


void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  initWiFi();         
  initRTC(TIME_ZONE); 
  initSDCard();       
  initFlashLight(PIN_FLASH_LIGHT);   
  setFlashBrightness(5);
  initCamera();       
  adjustSensorParams();
  composeFilename(dataPath, SD_DATA_DIR, "data", ".txt");

  printDateTime(2);
  printSystemInfo();
  printConnectionDetails();
  printSDCardInfo();
}

void loop() 
{
  const int msPeriod = 1000;
  const int msPulsewidth = 50;
  const int msFlashPeriod = 3000;
  const int msFlashPulsewidth = 2;
  const uint32_t msToWait = 5000; // output date and time every 5 seconds
  static uint32_t msPrevious = millis();

  // Blink the builtin red led every second for 50 ms
  digitalWrite(LED_BUILTIN, (millis()) % msPeriod < msPulsewidth ? LOW : HIGH);

  // Flash the builtin white led every 3 seconds for 2 ms
  flashLed(msFlashPeriod, msFlashPulsewidth);

  // Print the current date and time
  // and append a timestamp to a file
  if (waitIsOver(msPrevious, msToWait)) 
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