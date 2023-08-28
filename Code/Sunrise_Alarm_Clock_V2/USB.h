#ifndef USB_H
#define USB_H

#include<Arduino.h>
#include <TimeLib.h>

#include "Device.h"

class USB : public Device
{
  private:
    uint8_t pinNo;
    
  public:
    USB(uint8_t pinNo);

    void turnOn();
    void turnOff();

    void manageOutput(time_t t);
};

#endif // USB_H