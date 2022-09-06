
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <Arduino.h>

class Button{
public:
  int buttonPin;                                // navigation button pin
  
  int buttonOutput = 0;                         // navigation button analogue output value

  int buttonValue;                              // button value
  
  int buttonTolerance;                          // button tolerance (+/-)
  
  int buttonState;                              // Current state of the button
  int lastButtonState = LOW;                    // The last state of the button
  
  unsigned long stateChangeTime = 0;            // When the button changed state
  
  unsigned long debounceDelay = 200;            // debounce time

  Button(int _pin, int _value, int _tolerance);

  bool buttonIsPressed();
};


#endif // _BUTTON_H_
