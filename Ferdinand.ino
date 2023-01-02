#include <stdint.h>
#include <LiquidCrystal.h>
#include "hardware.h"
#include "battery.h"
#include "testlog.h"
#include "timer.h"

#define HOME_SCREEN 0
#define RESULTS_SCREEN 1
#define SETUP_SCREEN 2
#define	TEST_SCREEN 3

int screen = HOME_SCREEN;

void setup()
{
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
}

void loop()
{
    lcd.clear();
    switch (screen) {
	default:
	case HOME_SCREEN:
	    homeScreen();
	    break;
	case RESULTS_SCREEN:
	    resultsScreen();
	    break;
	case SETUP_SCREEN:
	    setupScreen();
	    break;
	case TEST_SCREEN:
	    testScreen();
	    break;
    }
}

void homeScreen()
{
    while (screen == HOME_SCREEN)
    {
        // draw the screen
        lcd.setCursor(0, 0);
        lcd.print("FRC 1425");
        lcd.setCursor(0, 1);
        lcd.print("Battery Test");
	lcd.noBlink();

        // wait for button release (without refreshing screen)
	int button;
	while ((button = readButtons()) != BUTTON_NONE)
	    ;

	// wait for button press
	while ((button = readButtons()) == BUTTON_NONE)
	    ;

	// take action
	switch (button) {
	    case BUTTON_RIGHT:
		screen = RESULTS_SCREEN;
		break;  
	}
    }
}

// draw the screen
void showEntry(LogEntry& entry)
{
    char buf[7];
    const char *stateName[] = {
	"BACK      ",
	"START     ",
	"RUNNING   ",
	"HALTED    ",
	"FINISHED  ",
    };
    lcd.setCursor(0, 0);
    lcd.print(entry.id);
    for (int col = strlen(entry.id); col < 10; col++)
	lcd.print(" ");
    lcd.print(mmss(buf, entry.time));
    lcd.setCursor(0, 1);
    lcd.print(stateName[entry.state]);
    lcd.print(vString(buf, (entry.state == LOG_FINISHED ? entry.vstart : entry.vend)));
}

void resultsScreen()
{
    int testNum = 0;

    while (screen == RESULTS_SCREEN)
    {
	int slot = TestLog.first() + testNum;
	if (slot >= TestLog.num_slots())
	    slot -= TestLog.num_slots();

	// draw the screen
	if (slot == TestLog.last()) {
	    lcd.setCursor(0, 0);
	    lcd.print("Log Empty ");
	    lcd.noBlink();
	} else {
	    LogEntry entry;
	    TestLog.get(slot, entry);
	    showEntry(entry);
	    lcd.noBlink();
	}

        // wait for button release (without refreshing screen)
	int button;
	while ((button = readButtons()) != BUTTON_NONE)
	    ;

	// wait for button press
	while ((button = readButtons()) == BUTTON_NONE)
	    ;

	// take action
	switch (button) {
	    case BUTTON_UP:
		if (slot != TestLog.last()) {
		    if (++slot >= TestLog.num_slots())
			slot -= TestLog.num_slots();
		    if (slot != TestLog.last())
			++testNum;
		}
		break;
	    case BUTTON_DOWN:
		if (testNum > 0)
		    --testNum;
		break;
	    case BUTTON_LEFT:
		screen = HOME_SCREEN;
		break;
	    case BUTTON_RIGHT:
		screen = SETUP_SCREEN;
		break;  
	}
    }
}

// extra 'Z' at beginning and ' ' at end to handle
// idPrev(' ') and idNext('Z') without messy special cases
const char idChars[] = "Z 0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

char idNext(char c)
{
    char *ptr = strchr(idChars + 1, c);
    if (ptr)
	return *++ptr;
    else
	return c;
}
 
char idPrev(char c)
{
    char *ptr = strchr(idChars + 1, c);
    if (ptr)
	return *--ptr;
    else
	return c;
}
 

void setupScreen()
{
    LogEntry entry;
    entry.state = LOG_NONE;
    strncpy(entry.id, "2020-00", sizeof entry.id);
    entry.vstart = 0;
    entry.vend = 1050;
    entry.time = 0;

    int cursor = 0;

    while (screen == SETUP_SCREEN)
    {
	// draw the screen
	showEntry(entry);

	if (cursor == -1) {
	    // allow selection of BACK or START
	    lcd.setCursor(0, 1);
	    lcd.blink();
	} else {
	    // add an underscore cursor below the selected ID character
	    lcd.setCursor(cursor, 0);
	    lcd.blink();
	}

        // wait for button release (without refreshing screen)
	int button;
	while ((button = readButtons()) != BUTTON_NONE)
	    ;

	// wait for button press
	while ((button = readButtons()) == BUTTON_NONE)
	    ;

	// take action
	switch (button) {
	    case BUTTON_UP:
#ifdef SERIAL_DEBUG
		Serial.println("BUTTON_UP");
#endif
		if (cursor == -1) {
		    entry.state = LOG_START;  // should this toggle?
		} else {
		    entry.id[cursor] = idNext(entry.id[cursor]);
		}
		break;
	    case BUTTON_DOWN:
#ifdef SERIAL_DEBUG
		Serial.println("BUTTON_DOWN");
#endif
		if (cursor == -1) {
		    entry.state = LOG_NONE;	// should this toggle?
		} else {
		    entry.id[cursor] = idPrev(entry.id[cursor]);
		}
		break;
	    case BUTTON_LEFT:
#ifdef SERIAL_DEBUG
		Serial.print("BUTTON_LEFT ");
		Serial.print(cursor);
		Serial.print("--> ");
#endif
		if (cursor >= 0) {
		    --cursor;
		} else {
		    cursor = sizeof(entry.id) - 2;
		}
#ifdef SERIAL_DEBUG
		Serial.println(cursor);
#endif
		break;
	    case BUTTON_RIGHT:
#ifdef SERIAL_DEBUG
		Serial.print("BUTTON_LEFT ");
		Serial.print(cursor);
		Serial.print("--> ");
#endif
		if (cursor < sizeof(entry.id) - 2) {
		    ++cursor;
		} else {
		    cursor = -1;
		}
#ifdef SERIAL_DEBUG
		Serial.println(cursor);
#endif
		break;
	    case BUTTON_SELECT:
		if (cursor == -1) {
		    screen = (entry.state == LOG_START) ? TEST_SCREEN : RESULTS_SCREEN;
		}
		break;
	}
    }
}

void startTest()
{
    // change mode/screen to "testing"
    // turn on load relay
    loadOn();
    // start timer
    startTimer();
}

void stopTest()
{
    // turn off load relay
    loadOff();
    // stop timer
    stopTimer();
    // record test result in EEPROM
}

void testScreen()
{
    bool running = false;

    // TODO: start the test

    while (screen == SETUP_SCREEN)
    {
        unsigned int vbat = 0;
	char buf[7];

        // draw the screen
        lcd.setCursor(0, 0);
        lcd.print(running ? "Running" : "Stopped");
        lcd.setCursor(0, 1);
        lcd.print(mmss(buf, testTime()));
        if (running) {
            vbat = battery();
#ifdef SERIAL_DEBUG      
            Serial.print(mmss(buf, testTime()));
            Serial.print(" ");
            Serial.println(vString(buf, vbat));
#endif
            lcd.setCursor(10, 1);
            lcd.print(vString(buf, vbat));
            
            if (vbat <= 1050) {
                running = false;
                stopTest();
                stopTimer();
                // need to log the test results here!
                // turn off power relay
                powerOff();
            }
        }    

        // wait for button release (without refreshing screen)
	int button;
	while ((button = readButtons()) != BUTTON_NONE)
	    ;

	// wait for button press
	while ((button = readButtons()) == BUTTON_NONE)
	    ;

	// take action
	switch (button) {
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
		screen = RESULTS_SCREEN;
		break;
	    case BUTTON_RIGHT:
		screen = HOME_SCREEN;
		break;
	}
    }
}
