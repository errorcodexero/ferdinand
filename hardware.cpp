#include <stdint.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "hardware.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int readButtons() {
    int adc_in = analogRead(BUTTONS);
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (adc_in < 70)   return BUTTON_RIGHT;  
    if (adc_in < 240)  return BUTTON_UP; 
    if (adc_in < 420)  return BUTTON_DOWN; 
    if (adc_in < 620)  return BUTTON_LEFT; 
    if (adc_in < 880)  return BUTTON_SELECT;   
    return BUTTON_NONE;  // when all others fail, return this...
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

