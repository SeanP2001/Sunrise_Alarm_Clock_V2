
#include "Device.h"

//---------------------------------------------------- C O N S T R U C T O R ----------------------------------------------------
Device::Device() : onTime(0), offTime(0) {}

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