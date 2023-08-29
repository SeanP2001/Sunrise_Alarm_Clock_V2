
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
  if(onTime < offTime)
  {
    if((hour(t) >= onTime) && (hour(t) < offTime))          // if the current hour is between the on and off hour
    {
      return true;
    }
  }
  if(onTime > offTime)
  {
    if((hour(t) >= onTime && hour(t) <= 23) || (hour(t) >= 0 && hour(t) < offTime))     // if it is after the on time at night or before the off time in the morning
    {
      return true;
    }
  }

  return false;;                 
}

//------------------------------------------------- I T   I S   O F F   T I M E -------------------------------------------------
bool Device::itIsOffTime(time_t t)
{
  return ((hour(t) == offTime) && (minute(t) == 0));                
}