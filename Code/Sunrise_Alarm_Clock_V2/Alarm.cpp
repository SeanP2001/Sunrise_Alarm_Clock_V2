
#include "Alarm.h"

//---------------------------------------------------- C O N S T R U C T O R ----------------------------------------------------
Alarm::Alarm(uint8_t pinNo) : Device(), pinNo(pinNo), enabled(false) {pinMode(pinNo, OUTPUT);}


//--------------------------------------------------------- E N A B L E ---------------------------------------------------------
void Alarm::enable()
{
  enabled = true;
}

//-------------------------------------------------------- D I S A B L E --------------------------------------------------------
void Alarm::disable()
{
  noTone(pinNo);

  enabled = false;
}

//---------------------------------------------------------- S O U N D ----------------------------------------------------------
void Alarm::sound()
{
  tone(pinNo, 2000);
  delay(1000);
  noTone(pinNo);
  delay(1000);   
}

//----------------------------------------------------- I S   E N A B L E D -----------------------------------------------------
bool Alarm::isEnabled()
{
  return enabled;
}

//--------------------------------------------------------- T O G G L E ---------------------------------------------------------
void Alarm::toggle()
{
  if(isEnabled())
  {
    disable();
  }
  else
  {
    enable();
  }
}

//-------------------------------------------------- M A N A G E   O U T P U T --------------------------------------------------
void Alarm::manageOutput(time_t t)
{
  if(itIsOnTime(t) && isEnabled())    // Turn on the light bar when it is scheduled                    
  {
    sound();                           
  }
  if(itIsOffTime(t))                
  {
    disable();
  }
}