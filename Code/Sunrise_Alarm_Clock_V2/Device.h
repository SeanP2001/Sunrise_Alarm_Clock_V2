#ifndef DEVICE_H
#define DEVICE_H

#include<Arduino.h>
#include <TimeLib.h>

class Device
{
  private:
    uint8_t onTime;
    uint8_t offTime;
    
  public:
    Device();

    void setOnTime(uint8_t hour);
    void setOffTime(uint8_t hour);

    bool itIsOnTime(time_t t);
    bool itIsOffTime(time_t t);
};

#endif // DEVICE_H