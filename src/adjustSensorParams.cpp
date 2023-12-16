#include <Arduino.h>
#include <esp_camera.h>
/**
 * Set the cameras sensor parameters
*/
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
