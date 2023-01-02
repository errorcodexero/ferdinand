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
TestStatus testStatus;

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
	lcd.noBlink();
        lcd.setCursor(0, 0);
        lcd.print("FRC 1425");
        lcd.setCursor(0, 1);
        lcd.print("Battery Test");

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
void showStatus()
{
    char buf[7];
    const char *stateName[] = {
	"BACK      ",
	"START     ",
	"RUNNING   ",
	"HALTED    ",
	"FINISHED  ",
    };
    lcd.noBlink();
    lcd.setCursor(0, 0);
    lcd.print(testStatus.id);
    for (int col = strlen(testStatus.id); col < 10; col++)
	lcd.print(" ");
    lcd.print(mmss(buf, testStatus.time));
    lcd.setCursor(0, 1);
    lcd.print(stateName[testStatus.state]);
    lcd.print(vString(buf, (testStatus.state == LOG_FINISHED ? testStatus.vstart : testStatus.vbat)));
}

void resultsScreen()
{
    int testNum = TestLog.num_used() - 1;

    while (screen == RESULTS_SCREEN)
    {
	// draw the screen
	if (testNum == -1) {
	    lcd.noBlink();
	    lcd.setCursor(0, 0);
	    lcd.print("Log Empty ");
	} else {
	    TestLog.get(testNum, testStatus);
	    showStatus();
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
		if (testNum < TestLog.num_used() - 1)
		    ++testNum;
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
    testStatus.state = LOG_NONE;
    strncpy(testStatus.id, "2023-01", sizeof testStatus.id);
    testStatus.vstart = testStatus.vbat = battery();
    testStatus.vend = 1050;
    testStatus.time = 0;

    int cursor = 0;

    while (screen == SETUP_SCREEN)
    {
	// draw the screen
	showStatus();

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
		    testStatus.state = LOG_START;  // should this toggle?
		} else {
		    testStatus.id[cursor] = idNext(testStatus.id[cursor]);
		}
		break;
	    case BUTTON_DOWN:
#ifdef SERIAL_DEBUG
		Serial.println("BUTTON_DOWN");
#endif
		if (cursor == -1) {
		    testStatus.state = LOG_NONE;	// should this toggle?
		} else {
		    testStatus.id[cursor] = idPrev(testStatus.id[cursor]);
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
		    cursor = sizeof(testStatus.id) - 2;
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
		if (cursor < sizeof(testStatus.id) - 2) {
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
		    if (testStatus.state == LOG_START) {
			// create a new log entry for this test
			TestLog.add(testStatus);
			// test screen will start the test
			screen = TEST_SCREEN;
		    } else {
			screen = RESULTS_SCREEN;
		    }
		}
		break;
	}
    }
}

void startTest()
{
    // turn on load relay
    loadOn();

    // start/continue timer
    startTimer(testStatus.time);

    // change status to "running"
    testStatus.state = LOG_RUNNING;
    TestLog.update(testStatus);
}

void stopTest(int why)
{
    // turn off load relay
    loadOff();

    // stop timer
    stopTimer();

    // record test result in EEPROM
    testStatus.state = why;
    TestLog.update(testStatus);
}

bool runTest()
{
    static bool tick = false;

    // update test status
    testStatus.time = testTime();
    testStatus.vbat = battery();
    showStatus();

    // log status every 60 seconds
    if ((testStatus.time % 60) == 0) {
	if (!tick)
	    TestLog.update(testStatus);
	tick = true;
    } else {
	tick = false;
    }

    // check for test complete
    if (testStatus.vbat <= testStatus.vend) {
	stopTest(LOG_FINISHED);
	// powerOff();
	return false;
    }
    return true;
}

void testScreen()
{
    startTest();
    bool running = true;

    while (screen == TEST_SCREEN)
    {
	// draw the screen
	showStatus();

        // wait for button release
	int button;
	while ((button = readButtons()) != BUTTON_NONE) {
	    // run the test while waiting
	    if (running) {
		if (!runTest()) {
		    running = false;
		    screen = RESULTS_SCREEN;
		    return;
		}
	    }
	}

	// wait for button press
	while ((button = readButtons()) == BUTTON_NONE) {
	    // run the test while waiting
	    if (running) {
		if (!runTest()) {
		    running = false;
		    screen = RESULTS_SCREEN;
		    return;
		}
	    }
	}

	// take action
	switch (button) {
	    case BUTTON_UP:
		if (!running) {
		    startTest();
		    running = true;
		}
		break;
	    case BUTTON_DOWN:
		if (running) {
		    stopTest(LOG_HALTED);
		    running = false;
		}
		break;
	    case BUTTON_LEFT:
		if (!running)
		    screen = RESULTS_SCREEN;
		break;
	}
    }
}
