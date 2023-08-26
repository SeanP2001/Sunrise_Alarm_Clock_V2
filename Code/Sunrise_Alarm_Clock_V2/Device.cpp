
#include "Device.h"

//---------------------------------------------------- C O N S T R U C T O R ----------------------------------------------------
Device::Device(uint8_t pinNo) : pinNo(pinNo), onTime(0), offTime(0), state(false) {pinMode(pinNo, OUTPUT);}

//---------------------------------------------------- S E T   O N   T I M E ----------------------------------------------------
void Device::setOnTime(uint8_t hour)
{
  if(hour < 24)
  {
    onTime = hour;
  }
}

//--------------------------------------------------- S E T   O F F   T I M E ---------------------------------------------------
void Device::setOffTime(uint8_t hour)
{
  if(hour < 24)
  {
    offTime = hour;
  }
}

//-------------------------------------------------------- T U R N   O N --------------------------------------------------------
void Device::turnOn()
{
  state = true;
  digitalWrite(pinNo, HIGH);
}

//------------------------------------------------------- T U R N   O F F -------------------------------------------------------
void Device::turnOff()
{
  state = false;
  digitalWrite(pinNo, LOW);
}

//---------------------------------------------------------- I S   O N ----------------------------------------------------------
bool Device::isOn()
{
  return state;
}

//--------------------------------------------------------- T O G G L E ---------------------------------------------------------
void Device::toggle()
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

//-------------------------------------------------- I T   I S   O N   T I M E --------------------------------------------------
bool Device::itIsOnTime(time_t t)
{
  return ((hour(t) >= onTime) && (hour(t) < offTime));                  
}

//------------------------------------------------- I T   I S   O F F   T I M E -------------------------------------------------
bool Device::itIsOffTime(time_t t)
{
  return ((hour(t) == offTime) && (minute(t) == 0));                
}

//-------------------------------------------------- M A N A G E   O U T P U T --------------------------------------------------
void Device::manageOutput(time_t t)
{
  if(itIsOnTime(t))    // Turn on the light bar when it is scheduled                    
  {
    turnOn();                            
  }
  if(itIsOffTime(t))                
  {
    turnOff();
  }
}