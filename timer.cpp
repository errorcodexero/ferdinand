#include <stdint.h>
#include <Arduino.h>
#include "hardware.h"
#include "timer.h"

static uint32_t timerVal = 0;

void startTimer()
{
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

void stopTimer()
{
    noInterrupts();
    TIMSK1 &= ~(1 << OCIE1A);  // enable timer compare interrupt
}

ISR(TIMER1_COMPA_vect)
{
    timerVal++;
}

void clearTimer()
{
    noInterrupts();
    timerVal = 0;
    interrupts();
}

uint32_t testTime()
{
    noInterrupts();
    uint32_t t = timerVal;
    interrupts();
    return t;
}

char *timeString(char hhmmss[], uint32_t t)
{
    uint16_t sec = t % 60;
    uint16_t min = (t / 60) % 60;
    uint16_t hr  = (t / 3600) % 100;
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

