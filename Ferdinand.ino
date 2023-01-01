#include <stdint.h>
#include <LiquidCrystal.h>
#include "hardware.h"
#include "battery.h"
#include "testlog.h"
#include "timer.h"

#define HOME_SCREEN 0
#define RESULTS_SCREEN 1
#define TEST_SCREEN 2

int screen = HOME_SCREEN;
bool released = true;

void setup() {
    // put your setup code here, to run once
    pinMode(BUTTONS, INPUT);
    pinMode(BATTERY, INPUT);
    pinMode(TEMP,    INPUT);
    pinMode(POWER,   OUTPUT);
    powerOff();
    pinMode(LOAD,    OUTPUT);
    loadOff();
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    powerOn();
#ifdef SERIAL_DEBUG
    Serial.begin(9600);
    Serial.println("Hello world\\n");
#endif
    TestLog.begin();
    {
	LogEntry entry1 = { LOG_FINISHED, "2023-01", 1320, 1050, 30*60 };
	LogEntry entry2 = { LOG_HALTED,   "2023-02", 1220, 1180, 20*60 };
	LogEntry entry3 = { LOG_RUNNING,  "2023-03", 1380, 1225,  5*60 };

	TestLog.put(0, entry1);
	TestLog.put(1, entry2);
	TestLog.put(2, entry3);
    }
}

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
	int slot = TestLog.first() + result;
	if (slot >= TestLog.num_slots()) {
	    slot -= TestLog.num_slots();
	}
	if (slot == TestLog.last()) {
	    // draw the screen
	    lcd.setCursor(0, 0);
	    lcd.print("No Tests Logged");
	} else {
	    LogEntry entry;
	    const char *stateName[] = {
		"NEW       ",
		"STARTED   ",
		"RUNNING   ",
		"HALTED    ",
		"FINISHED  ",
	    };
	    char hhmmss[9];
	    char vstr[7];

	    TestLog.get(slot, entry);

	    // draw the screen
	    lcd.setCursor(0, 0);
	    lcd.print(entry.id);
	    for (int col = strlen(entry.id); col < 8; col++)
		lcd.print(" ");
	    lcd.print(timeString(hhmmss, entry.time));

	    lcd.setCursor(0, 1);
	    lcd.print(stateName[entry.state]);
	    lcd.print(vString(vstr, (entry.state == LOG_FINISHED ? entry.vstart : entry.vend)));
	}
        // wait for button press
        switch (readButtons()) {
            case BUTTON_UP:
                if (released) {
		    if (result < TestLog.num_slots()) {
			if (++slot >= TestLog.num_slots()) {
			    slot -= TestLog.num_slots();
			}
			if (slot != TestLog.last()) {
			    ++result;
			}
		    }
                }
                released = false;
                break;
            case BUTTON_DOWN:
                if (released) {
		    if (result > 0) {
			--result;
		    }
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

void testScreen() {
    bool running = false;
    while (screen == TEST_SCREEN) {
        unsigned int vbat = 0;
	char hhmmss[9];
	char vstr[7];
        // draw the screen
        lcd.setCursor(0, 0);
        lcd.print(running ? "Running" : "Stopped");
        lcd.setCursor(0, 1);
        lcd.print(timeString(hhmmss, testTime()));
        if (running) {
            vbat = battery();
#ifdef SERIAL_DEBUG      
            Serial.print(timeString(hhmmss, testTime()));
            Serial.print(" ");
            Serial.println(vString(vstr, vbat));
#endif
            lcd.setCursor(10, 1);
            lcd.print(vString(vstr, vbat));
            
            if (vbat <= 1050) {
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
