#include <Arduino.h>

#include "AdvancedDigitalPin.h"
#include "AdvancedAnalogPin.h"
#include "HID-Project.h"

#define VOLUME_INPUT A3

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_SW 15

void detectJoystickMovement();
int getMappedJoystickValue(int pinValue);
int getMappedJoystickValueWithDelay(int index);
void adjustVolume();
void setVolumeToZero();
int scale(int value);
bool isFirstPress(AdvancedDigitalPin &b);
bool isHeld(AdvancedDigitalPin &b);


// Joystick
AdvancedDigitalPin joystickSW(JOYSTICK_SW, INPUT_PULLUP, 100);
AdvancedAnalogPin joystick_X(JOYSTICK_X, INPUT, 100);
AdvancedAnalogPin joystick_Y(JOYSTICK_Y, INPUT, 100);

int joystickMappedValues[] = {0, 1, 5, 15};
int repeatDelay = 250;
unsigned long lastRepeat = 0;

int lastVolume;

void setup() {
  Serial.begin(9600);

  Keyboard.begin();
  Mouse.begin();
  Consumer.begin();

  // Turn off TX and RX leds
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);

  pinMode(VOLUME_INPUT, INPUT);

  setVolumeToZero();
}

void loop() {
  adjustVolume();
  detectJoystickMovement();

  if (isFirstPress(joystickSW)) {
    setVolumeToZero();
  }
}

void detectJoystickMovement() {
  int xMov = getMappedJoystickValue(joystick_X.getPinValue());
  int yMov = getMappedJoystickValue(joystick_Y.getPinValue());

  // Scroll wheel Up/Down  
  if (xMov != 0 && joystick_X.hasTimePassed()) {
    joystick_X.updateLastReadTime();
    Mouse.move(0, 0, xMov);
  }
  
  // Left Shift + Scroll wheel Up/Down 
  if (yMov != 0 && joystick_Y.hasTimePassed()) {
    joystick_Y.updateLastReadTime();
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.move(0, 0, yMov);
    Keyboard.releaseAll();
  }
}

int getMappedJoystickValue(int pinValue) {
  if (pinValue < 0 || pinValue > 1023)
    return 0;

  if (pinValue <= 30) // [0,30] = 31
    return getMappedJoystickValueWithDelay(3);
  if (pinValue <= 100) // (30,100] = 70
    return getMappedJoystickValueWithDelay(2);
  else if (pinValue <= 250) // (100,250] = 150
    return getMappedJoystickValueWithDelay(1);
  else if (pinValue <= 490) // (250,490] = 240
    return getMappedJoystickValueWithDelay(0);
  else if (pinValue <= 530) // (490,530]  20--(511)--20 = 39
    return 0;
  else if (pinValue <= 770) // (530,770] = 240
    return -getMappedJoystickValueWithDelay(0);
  else if (pinValue <= 920) // (770,920] = 150
    return -getMappedJoystickValueWithDelay(1);
  else if (pinValue <= 990) // (920,990] = 70
    return -getMappedJoystickValueWithDelay(2);
  else // (990,1023] = 33
    return -getMappedJoystickValueWithDelay(3);  
}

int getMappedJoystickValueWithDelay(int index) {
  if (index > 3 || index < 0) {
    return 0;
  }

  int val = joystickMappedValues[index];

  if (val != 0) {
    return val;
  }
  
  if (lastRepeat <= millis() - repeatDelay) {
    lastRepeat = millis();
    return 1;
  }

  return 0;
}

void adjustVolume() {
  int volume = scale(analogRead(VOLUME_INPUT));
  if (lastVolume < volume) {
    Consumer.press(MEDIA_VOLUME_UP);
    Consumer.release(MEDIA_VOLUME_UP);
    lastVolume++;
    delay(10);
  } else if (lastVolume > volume) {
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    lastVolume--;
    delay(10);
  }
}

void setVolumeToZero() {
  lastVolume = 0;
  for (int i = 0; i < 100; i++) {
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    delay(10);
  }
}

int scale(int value) {
  return 50 - value * 50L / 1023;
}

/*
  Returns TRUE the first time the PIN reads LOW and then locks the pin until a HIGH is read.
  Allows PIN to trigger code ONLY ONCE while held.
  An additional check is made before checking the PIN status. This check is whether or not enough time has passed since the last STATE change
*/
bool isFirstPress(AdvancedDigitalPin &b) {
  if (b.hasStateChangedTooFast())
    return false;
    
  int sensorVal = b.getCurrentPinState();
  if (b.getState() == sensorVal)
    return false;

  if (sensorVal == LOW) {
    b.setState(LOW);
    return true;
  } 
  else if (sensorVal == HIGH && b.getState() == LOW) {
    b.setState(HIGH);
  }
  return false;  
}

/*
  Returns TRUE the first time the PIN reads LOW and then locks the pin until a HIGH is read.
  Allows PIN to trigger code ONLY ONCE while held.
  An additional check is made before checking the PIN status. This check is whether or not enough time has passed since the last STATE change
*/
bool isHeld(AdvancedDigitalPin &b) {
  if (b.hasStateChangedTooFast())
    return false;
    
  int sensorVal = b.getCurrentPinState();
  if (b.getState() == sensorVal && sensorVal == LOW)
    return true;

  if (sensorVal == LOW) {
    b.setState(LOW);
    return true;
  } 
  else if (sensorVal == HIGH && b.getState() == LOW) {
    b.setState(HIGH);
  }
  return false;
}