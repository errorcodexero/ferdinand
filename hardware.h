#include <stdint.h>
#include <LiquidCrystal.h>

#define SERIAL_DEBUG

#define BUTTONS A0
#define TEMP    A1
#define BATTERY A2
#define LOAD    2
#define POWER   3

extern LiquidCrystal lcd;

#define BUTTON_NONE   0
#define BUTTON_SELECT 1
#define BUTTON_UP     2
#define BUTTON_LEFT   3
#define BUTTON_RIGHT  4
#define BUTTON_DOWN   5

extern int readButtons();

extern void powerOn();
extern void powerOff();
extern void loadOn();
extern void loadOff();

