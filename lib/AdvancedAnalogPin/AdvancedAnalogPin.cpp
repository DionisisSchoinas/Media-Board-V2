#include "Arduino.h"
// AdvancedAnalogPin.cpp
#include "AdvancedAnalogPin.h"

AdvancedAnalogPin::AdvancedAnalogPin(int analogPin, int analogPinMode, unsigned short minReadTime) {
  this-> analogPin = analogPin;
  pinMode(analogPin, analogPinMode);
  this-> minReadTime = minReadTime;
  updateLastReadTime();
}

void AdvancedAnalogPin::updateLastReadTime() {
  lastReadTime = millis();
}

bool AdvancedAnalogPin::hasTimePassed() {
  return (millis() - lastReadTime) >= minReadTime;
}

int AdvancedAnalogPin::getPinValue() {
  return analogRead(analogPin);
}

void AdvancedAnalogPin::print() {
  Serial.print("analogPin ");
  Serial.print(analogPin);
  Serial.print(" getPinValue ");
  Serial.println(getPinValue());
}