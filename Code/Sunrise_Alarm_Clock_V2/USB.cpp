#include "USB.h"

//---------------------------------------------------- C O N S T R U C T O R ----------------------------------------------------
USB::USB(uint8_t pinNo) : Device(), pinNo(pinNo) {pinMode(pinNo, OUTPUT);}

//-------------------------------------------------------- T U R N   O N --------------------------------------------------------
void USB::turnOn()
{
  digitalWrite(pinNo, HIGH);
}

//------------------------------------------------------- T U R N   O F F -------------------------------------------------------
void USB::turnOff()
{
  digitalWrite(pinNo, LOW);
}

//-------------------------------------------------- M A N A G E   O U T P U T --------------------------------------------------
void USB::manageOutput(time_t t)
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