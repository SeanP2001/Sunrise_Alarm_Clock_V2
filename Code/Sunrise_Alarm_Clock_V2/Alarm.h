#ifndef ALARM_H
#define ALARM_H

#include<Arduino.h>
#include <TimeLib.h>

#include "Device.h"

class Alarm : public Device
{
  private:
    uint8_t pinNo;
    bool enabled;

  public:
    Alarm(uint8_t pinNo);

    void enable();
    void disable();
  
    void sound();

    bool isEnabled();

    void toggle();

    void manageOutput(time_t t);
};

#endif // ALARM_H