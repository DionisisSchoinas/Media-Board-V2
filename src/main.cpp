#include <Arduino.h>

#include "AdvancedDigitalPin.h"
#include "AdvancedAnalogPin.h"
#include "HID-Project.h"
#include <Smooth.h>

#define SERIAL_PRINT 0

#define VOLUME_INPUT A3

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_SW 15

#define VOLUME_ANALOG_READ_DEADZONE 2
#define VOLUME_SCALE_DEADZONE_MIN 75
#define VOLUME_SCALE_DEADZONE_MAX 950
#define VOLUME_SCALE_OUTPUT_MIN 0
#define VOLUME_SCALE_OUTPUT_MAX 50

void detectJoystickMovement();
int getMappedJoystickValue(int pinValue);
int getMappedJoystickValueWithDelay(int index);
void adjustVolume();
void setVolumeToZero();
short scale(int value);
int getAnalogReadAfterDeadZone(int value);
bool isFirstPress(AdvancedDigitalPin &b);
bool isHeld(AdvancedDigitalPin &b);


// Joystick
AdvancedDigitalPin joystickSW(JOYSTICK_SW, INPUT_PULLUP, 100);
AdvancedAnalogPin joystick_X(JOYSTICK_X, INPUT, 100);
AdvancedAnalogPin joystick_Y(JOYSTICK_Y, INPUT, 100);

int joystickMappedValues[] = {0, 1, 5, 15};
int repeatDelay = 250;
unsigned long lastRepeat = 0;


Smooth averageVolume(200);
short volume;
short lastVolume;
int lastAnalogRead;

void setup() {
  #if SERIAL_PRINT
  Serial.begin(9600);
  #endif

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
  averageVolume.add(getAnalogReadAfterDeadZone(analogRead(VOLUME_INPUT)));

  lastAnalogRead = averageVolume.get_avg();

  #if SERIAL_PRINT
  Serial.print(">finalValue:");
  Serial.println(lastAnalogRead);
  #endif

  volume = 50 - scale(lastAnalogRead);

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
  volume = 0;
  lastVolume = 0;
  lastAnalogRead = 0;
  averageVolume.set_avg(VOLUME_SCALE_DEADZONE_MAX);
  for (int i = 0; i < 100; i++) {
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    delay(10);
  }
}

/*
  y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
  x E 23-1000
  y E 0-50
*/
short scale(int value) {
  if (value < VOLUME_SCALE_DEADZONE_MIN) {
    return VOLUME_SCALE_OUTPUT_MIN;
  } else if (value > VOLUME_SCALE_DEADZONE_MAX) {
    return VOLUME_SCALE_OUTPUT_MAX;
  }
  return (long)VOLUME_SCALE_OUTPUT_MAX * (value-VOLUME_SCALE_DEADZONE_MIN) / (VOLUME_SCALE_DEADZONE_MAX-VOLUME_SCALE_DEADZONE_MIN);
}

/*
  If given value is between the last value +- a specified range,
  then ignore the new value and return the old value
  else return the given value.
*/
int getAnalogReadAfterDeadZone(int value) {
  #if SERIAL_PRINT
  Serial.print(">newValue:");
  Serial.println(value);
  #endif

  if (value <= lastAnalogRead + VOLUME_ANALOG_READ_DEADZONE && 
    value >= lastAnalogRead - VOLUME_ANALOG_READ_DEADZONE
  ) {
    return lastAnalogRead;
  }
  return value;
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