#include <stdint.h>
#include <Arduino.h>
#include "hardware.h"
#include "timer.h"

static uint32_t timerVal = 0;

void startTimer(uint32_t start_time)
{
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 62500;            // compare match register 16MHz/256/1Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

    timerVal = start_time;

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

char *hhmmss(char buf[9], uint32_t t)
{
    uint16_t sec = t % 60;
    uint16_t min = (t / 60) % 60;
    uint16_t hr  = (t / 3600) % 100;
    buf[0] = '0' + (hr / 10);
    buf[1] = '0' + (hr % 10);
    buf[2] = ':';
    buf[3] = '0' + (min / 10);
    buf[4] = '0' + (min% 10);
    buf[5] = ':';
    buf[6] = '0' + (sec / 10);
    buf[7] = '0' + (sec % 10);
    buf[8] = '\0';
    return buf;
}

char *mmss(char buf[7], uint32_t t)
{
    uint16_t sec = t % 60;
    uint16_t min = (t / 60) % 100;
    buf[0] = '0' + (min / 10);
    buf[1] = '0' + (min% 10);
    buf[2] = 'm';
    buf[3] = '0' + (sec / 10);
    buf[4] = '0' + (sec % 10);
    buf[5] = 's';
    buf[6] = '\0';
    return buf;
}

