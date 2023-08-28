#ifndef LIGHT_H
#define LIGHT_H

#include<Arduino.h>
#include <TimeLib.h>

#include "Device.h"

class Light : public Device
{
  private:
    uint8_t pinNo;
    bool state;
    
  public:
    Light(uint8_t pinNo);

    void turnOn();
    void turnOff();

    bool isOn();

    void toggle();

    void manageOutput(time_t t);
};

#endif // LIGHT_H