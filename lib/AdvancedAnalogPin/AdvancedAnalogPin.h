// AdvancedAnalogPin.h
#ifndef AdvancedAnalogPin_h
#define AdvancedAnalogPin_h

#include "Arduino.h"

class AdvancedAnalogPin {
  private:
    int analogPin;
    unsigned long lastReadTime;
    unsigned short minReadTime;

  public:
    AdvancedAnalogPin(int analogPin, int analogPinMode, unsigned short minReadTime);
    void updateLastReadTime();
    bool hasTimePassed();
    int getPinValue();
    void print();
};

#endif