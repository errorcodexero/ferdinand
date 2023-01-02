#include <stdint.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "hardware.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int readButtonsOnce() {
    int adc_in = analogRead(BUTTONS);
    if (adc_in < 70)   return BUTTON_RIGHT;  
    if (adc_in < 240)  return BUTTON_UP; 
    if (adc_in < 420)  return BUTTON_DOWN; 
    if (adc_in < 620)  return BUTTON_LEFT; 
    if (adc_in < 880)  return BUTTON_SELECT;   
    return BUTTON_NONE;  // when all others fail, return this...
}

int readButtons() {
    int b[4] = { -1, -2, -3, -4 };
    int i = 0;
    while (b[0] != b[1] || b[0] != b[2] || b[0] != b[3]) {
	delay(5);
	b[i] = readButtonsOnce();
	if (++i > 3) i = 0;
    }
    return b[0];
}

void powerOn() {
    digitalWrite(POWER, HIGH);
}

void powerOff() {
    digitalWrite(POWER, LOW);
}

void loadOn() {
    digitalWrite(LOAD, HIGH);
}

void loadOff() {
    digitalWrite(LOAD, LOW);
}

