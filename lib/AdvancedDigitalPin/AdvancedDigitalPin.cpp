// AdvancedDigitalPin.cpp
#include "AdvancedDigitalPin.h"

AdvancedDigitalPin::AdvancedDigitalPin(unsigned short minStateHoldTime) {
  this-> minStateHoldTime = minStateHoldTime;
  setState(0);
}

AdvancedDigitalPin::AdvancedDigitalPin(int digitalPin, int digitalPinMode) {
  this-> digitalPin = digitalPin;
  this-> minStateHoldTime = 0;
  pinMode(digitalPin, digitalPinMode);
  setState();
}

AdvancedDigitalPin::AdvancedDigitalPin(int digitalPin, int digitalPinMode, unsigned short minStateHoldTime) : AdvancedDigitalPin(digitalPin, digitalPinMode) {
  this-> minStateHoldTime = max(minStateHoldTime, 0);
}

bool AdvancedDigitalPin::hasTimePassed() {
  return (millis() - lastStateChangeTime) >= minStateHoldTime;
}

void AdvancedDigitalPin::updateLastStateChangeTime() {
  if (minStateHoldTime == 0)
    return;
  lastStateChangeTime = millis();
}

bool AdvancedDigitalPin::hasStateChangedTooFast() {
  return !hasTimePassed() && (getState() != getCurrentPinState());
}

void AdvancedDigitalPin::setState() {
  state = getCurrentPinState();
  updateLastStateChangeTime();
}

void AdvancedDigitalPin::setState(int newState) {
  state = newState;
  updateLastStateChangeTime();
}

int AdvancedDigitalPin::getState() {
  return state;
}

int AdvancedDigitalPin::getCurrentPinState() {
  return digitalRead(digitalPin);
}

int AdvancedDigitalPin::getDigitalPin() {
  return digitalPin;
}

void AdvancedDigitalPin::print() {
  Serial.print("digitalPin ");
  Serial.print(digitalPin);
  Serial.print(" state ");
  Serial.print(state);
  Serial.print(" lastStateChangeTime ");
  Serial.print(lastStateChangeTime);
  Serial.print(" hasTimePassed ");
  Serial.print(hasTimePassed());
  Serial.print(" getCurrentPinState ");
  Serial.print(getCurrentPinState());
  Serial.print(" hasStateChangedTooFast ");
  Serial.println(hasStateChangedTooFast());
}