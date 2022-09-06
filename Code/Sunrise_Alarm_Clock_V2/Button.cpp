
#include "Button.h"

Button::Button(int _pin, int _value, int _tolerance)
{
  this->buttonPin = _pin;
  this->buttonValue = _value;
  this->buttonTolerance = _tolerance;
}


bool Button::buttonIsPressed()
{
  buttonState = LOW;                                                                                                              // the button is low by default
   
  buttonOutput = analogRead(buttonPin);                                                                                 // read the button board output
  
  if ((buttonOutput >= (buttonValue - buttonTolerance)) && (buttonOutput <= (buttonValue + buttonTolerance)))           // if it is in the button range, set the button state HIGH
    buttonState = HIGH;
  

  if ((buttonState != lastButtonState)&&(millis()>(stateChangeTime + debounceDelay)))           // if the button has changed state and the debounce delay has passed
  {
    if (buttonState == HIGH)                                                                    // if it changed to HIGH
    {
      stateChangeTime = millis();
      return true;                                                                              // return TRUE
    }
  }
  lastButtonState = buttonState;                                                                // log the button state
  
  return false;                                                                                 // otherwise, return FALSE                                                         
}
