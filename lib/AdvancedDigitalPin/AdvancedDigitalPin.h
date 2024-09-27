// AdvancedDigitalPin.h
#ifndef AdvancedDigitalPin_h
#define AdvancedDigitalPin_h

#include "Arduino.h"

class AdvancedDigitalPin {
  private:
    int digitalPin;
    int state;
    unsigned long lastStateChangeTime;
    unsigned short minStateHoldTime;
    void updateLastStateChangeTime();

  public:
    AdvancedDigitalPin(int digitalPin, int pinMode, unsigned short minStateHoldTime);
    AdvancedDigitalPin(int digitalPin, int pinMode);
    AdvancedDigitalPin(unsigned short minStateHoldTime);
    bool hasTimePassed();
    bool hasStateChangedTooFast();
    void setState();
    void setState(int newState);
    int getState();
    int getCurrentPinState();
    int getDigitalPin();
    void print();
};

#endif