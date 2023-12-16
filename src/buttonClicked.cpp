#include <Arduino.h>
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