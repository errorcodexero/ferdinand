#include <EEPROM.h>
#include <LiquidCrystal.h>

// #include <stdtypes.h>
typedef unsigned int u_int;
typedef unsigned long u_long;

#define BUTTONS A0
#define TEMP    A1
#define BATTERY A2
#define LOAD    2
#define POWER   3

#define VCC     4.96
#define R5      1000.
#define R6      510.      

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

u_long timerVal = 0;

#define BUTTON_NONE   0
#define BUTTON_SELECT 1
#define BUTTON_UP     2
#define BUTTON_LEFT   3
#define BUTTON_RIGHT  4
#define BUTTON_DOWN   5

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

void startTimer() {
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 62500;            // compare match register 16MHz/256/1Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

void stopTimer() {
  noInterrupts();
  TIMSK1 &= ~(1 << OCIE1A);  // enable timer compare interrupt
}

ISR(TIMER1_COMPA_vect) {
  timerVal++;
}

void clearTimer() {
  noInterrupts();
  timerVal = 0;
  interrupts();
}

u_long testTime() {
  noInterrupts();
  u_long t = timerVal;
  interrupts();
  return t;
}

char *timeString(u_long t)
{
  static char hhmmss[9];
  u_int sec = t % 60;
  u_int min = (t / 60) % 60;
  u_int hr  = (t / 3600) % 100;
  hhmmss[0] = '0' + (hr / 10);
  hhmmss[1] = '0' + (hr % 10);
  hhmmss[2] = ':';
  hhmmss[3] = '0' + (min / 10);
  hhmmss[4] = '0' + (min% 10);
  hhmmss[5] = ':';
  hhmmss[6] = '0' + (sec / 10);
  hhmmss[7] = '0' + (sec % 10);
  hhmmss[8] = '\0';
  return hhmmss;
}

void startTest() {
  // change mode/screen to "testing"
  // turn on load relay
  loadOn();
  // start timer
  startTimer();
}

void stopTest() {
  // turn off load relay
  loadOff();
  // stop timer
  stopTimer();
  // record test result in EEPROM
}

float battery() {
  const float scale = VCC * (R5 + R6) / (1024. * R6);
  int ain = analogRead(BATTERY);
  float vbat = ain * scale;
  return vbat;
}

char *vString(float v)
{
  static char vstr[7];
  int v100 = (int)(v * 100.0 + 0.5);
  vstr[0] = (v100 >= 1000) ? '1' : ' ';
  vstr[1] = '0' + (v100 / 100) % 10;
  vstr[2] = '.';
  vstr[3] = '0' + (v100 / 10) % 10;
  vstr[4] = '0' + v100 % 10;
  vstr[5] = 'V';
  vstr[6] = '\0';
  return vstr;
}
  
void setup() {
  // put your setup code here, to run once
  pinMode(BUTTONS, INPUT);
  pinMode(BATTERY, INPUT);
  pinMode(TEMP, INPUT);
  pinMode(POWER, OUTPUT);
  powerOff();
  pinMode(LOAD, OUTPUT);
  loadOff();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  delay(100);  // 100mS
  powerOn();
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
#endif
}

#define HOME_SCREEN 0
#define RESULTS_SCREEN 1
#define TEST_SCREEN 2

int screen = HOME_SCREEN;

#define NUM_RESULTS 4

bool released = true;


void loop() {
    lcd.clear();
    switch (screen) {
        case HOME_SCREEN:
            homeScreen();
            break;
        case RESULTS_SCREEN:
            resultsScreen();
            break;
        case TEST_SCREEN:
            testScreen();
            break;
        
    }
}

void homeScreen() {
  while (screen == HOME_SCREEN) {
    // draw the screen
    lcd.setCursor(0, 0);
    lcd.print("FRC 1425");
    lcd.setCursor(0, 1);
    lcd.print("Battery Test");
    // wait for button press
    switch (readButtons()) {
      case BUTTON_LEFT:
        if (released)
          screen = TEST_SCREEN;
        released = false;
        break;
      case BUTTON_RIGHT:
        if (released)
          screen = RESULTS_SCREEN;
        released = false;
        break;  
      case BUTTON_NONE:
        released = true;
        break;
    }
  }
}

void resultsScreen() {
  int result = 0;
  while (screen == RESULTS_SCREEN) {
    // draw the screen
    lcd.setCursor(0, 0);
    lcd.print("Result ");
    lcd.print(result);
    lcd.setCursor(0, 1);
    lcd.print("some numbers");
    // wait for button press
    switch (readButtons()) {
      case BUTTON_UP:
        if (released) {
            if (++result >= NUM_RESULTS)
              result -= NUM_RESULTS;
        }
        released = false;
        break;
      case BUTTON_DOWN:
        if (released) {
            if (--result < 0)
              result += NUM_RESULTS;
        }
        released = false;      
        break;
      case BUTTON_LEFT:
        if (released)
          screen = HOME_SCREEN;
        released = false;
        break;
      case BUTTON_RIGHT:
        if (released)
          screen = TEST_SCREEN;
        released = false;
        break;  
      case BUTTON_NONE:
        released = true;
        break;
    }
  }
}

void testScreen() {
  bool running = false;
  while (screen == TEST_SCREEN) {
    float vbat = 0.0;
    // draw the screen
    lcd.setCursor(0, 0);
    lcd.print(running ? "Running" : "Stopped");
    lcd.setCursor(0, 1);
    lcd.print(timeString(testTime()));
    if (running) {
      vbat = battery();
#ifdef SERIAL_DEBUG      
      Serial.print(timeString(testTime()));
      Serial.print(" ");
      Serial.println(vString(vbat));
#endif
      lcd.setCursor(10, 1);
      lcd.print(vString(vbat));
      
      if (vbat <= 10.50) {
        running = false;
        stopTest();
        stopTimer();
        // need to log the test results here!
        // turn off power relay
        powerOff();
      }
    }    
    // wait for button press
    switch (readButtons()) {
      case BUTTON_UP:
        if (!running) {
            startTest();
            startTimer();
            running = true;
        }
        break;
      case BUTTON_DOWN:
        if (running) {
            stopTest();
            stopTimer();
            running = false;
        }
        break;
      case BUTTON_LEFT:
        if (released)
          screen = RESULTS_SCREEN;
        released = false;
        break;
      case BUTTON_RIGHT:
        if (released)
          screen = HOME_SCREEN;
        released = false;
        break;
      case BUTTON_NONE:
        released = true;
        break;
    }
  }
}
