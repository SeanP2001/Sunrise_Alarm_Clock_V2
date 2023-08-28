#include "Light.h"

//---------------------------------------------------- C O N S T R U C T O R ----------------------------------------------------
Light::Light(uint8_t pinNo) : Device(), pinNo(pinNo), state(false) {pinMode(pinNo, OUTPUT);}

//-------------------------------------------------------- T U R N   O N --------------------------------------------------------
void Light::turnOn()
{
  state = true;
  digitalWrite(pinNo, HIGH);
}

//------------------------------------------------------- T U R N   O F F -------------------------------------------------------
void Light::turnOff()
{
  state = false;
  digitalWrite(pinNo, LOW);
}

//---------------------------------------------------------- I S   O N ----------------------------------------------------------
bool Light::isOn()
{
  return state;
}

//--------------------------------------------------------- T O G G L E ---------------------------------------------------------
void Light::toggle()
{
  if(isOn())
  {
    turnOff();
  }
  else
  {
    turnOn();
  }
}

//-------------------------------------------------- M A N A G E   O U T P U T --------------------------------------------------
void Light::manageOutput(time_t t)
{
  if(itIsOnTime(t))    // Turn on the USB port when it is scheduled                    
  {
    turnOn();                            
  }
  if(itIsOffTime(t))                
  {
    turnOff();
  }
}