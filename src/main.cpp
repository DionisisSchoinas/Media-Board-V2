#include <Arduino.h>

#include "AdvancedAnalogPin.h"
#include "HID-Project.h"
#include "OneButton.h"

#define SERIAL_PRINT 0

#define BUTTON_1_PIN 3
#define BUTTON_2_PIN 4
#define BUTTON_3_PIN 5
#define BUTTON_4_PIN 6
#define BUTTON_5_PIN 7
#define BUTTON_6_PIN 8
#define BUTTON_7_PIN 9

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_SW 15

void detectJoystickMovement();
int getMappedJoystickValue(int pinValue);
int getMappedJoystickValueWithDelay(int index);
void tickButtons();
void setVolumeToZero();
void button3Click();
void button3LongPress();
void button4Click();
void button5Click();
void button5LongPress();

// Buttons
OneButton button1(BUTTON_1_PIN);
OneButton button2(BUTTON_2_PIN);
OneButton button3(BUTTON_3_PIN);
OneButton button4(BUTTON_4_PIN);
OneButton button5(BUTTON_5_PIN);
OneButton button6(BUTTON_6_PIN);
OneButton button7(BUTTON_7_PIN);

// Joystick
OneButton joystickSW(JOYSTICK_SW);
AdvancedAnalogPin joystick_X(JOYSTICK_X, INPUT, 100);
AdvancedAnalogPin joystick_Y(JOYSTICK_Y, INPUT, 100);

int joystickMappedValues[] = {0, 1, 5, 15};
int repeatDelay = 250;
unsigned long lastRepeat = 0;

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

  joystickSW.attachClick(setVolumeToZero);

  button3.setClickMs(200);
  button3.setPressMs(400);
  button3.attachClick(button3Click);
  button3.attachDuringLongPress(button3LongPress);
  
  button4.attachClick(button4Click);

  button5.setClickMs(200);
  button5.setPressMs(400);
  button5.attachClick(button5Click);
  button5.attachDuringLongPress(button5LongPress);
}

void loop() {
  tickButtons();
  detectJoystickMovement();
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

void tickButtons() {
  button1.tick();
  button2.tick();
  button3.tick();
  button4.tick();
  button5.tick();
  button6.tick();
  button7.tick();
  joystickSW.tick();
}

void setVolumeToZero() {
  for (int i = 0; i < 100; i++) {
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    delay(10);
  }
}

void button3Click() {
    #if SERIAL_PRINT
      Serial.println("Button 3 pressed");
    #endif
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
}

void button3LongPress() {
    #if SERIAL_PRINT
      Serial.println("Button 3 Long Press");
    #endif
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    delay(50);
}

void button4Click() {
    #if SERIAL_PRINT
      Serial.println("Button 4 pressed");
    #endif
    Consumer.press(MEDIA_VOLUME_MUTE);
    Consumer.release(MEDIA_VOLUME_MUTE);
}

void button5Click() {
    #if SERIAL_PRINT
      Serial.println("Button 5 pressed");
    #endif
    Consumer.press(MEDIA_VOLUME_UP);
    Consumer.release(MEDIA_VOLUME_UP);
}

void button5LongPress() {
    #if SERIAL_PRINT
      Serial.println("Button 5 Long Press");
    #endif
    Consumer.press(MEDIA_VOLUME_UP);
    Consumer.release(MEDIA_VOLUME_UP);
    delay(50);
}