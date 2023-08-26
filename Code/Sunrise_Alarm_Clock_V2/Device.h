#ifndef DEVICE_H
#define DEVICE_H

#include<Arduino.h>
#include <TimeLib.h>

class Device
{
  private:
    uint8_t pinNo;
    uint8_t onTime;
    uint8_t offTime;
    bool state;
    
  public:
    Device(uint8_t pinNo);

    void setOnTime(uint8_t hour);
    void setOffTime(uint8_t hour);

    void turnOn();
    void turnOff();

    bool isOn();

    void toggle();

    bool itIsOnTime(time_t t);
    bool itIsOffTime(time_t t);

    void manageOutput(time_t t);
};

#endif // DEVICE_H