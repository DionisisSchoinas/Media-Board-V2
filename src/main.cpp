#include <Arduino.h>

#include "HID-Project.h"

#define VOLUME_INPUT A3

int scale(int value);
void adjustVolume();
void setVolumeToZero();

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

  lastVolume = 0;
  setVolumeToZero();
}

void loop() {
  adjustVolume();
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
  for (int i = 0; i < 100; i++) {
    Consumer.press(MEDIA_VOLUME_DOWN);
    Consumer.release(MEDIA_VOLUME_DOWN);
    delay(10);
  }
}

int scale(int value) {
  return 50 - value * 50L / 1023;
}