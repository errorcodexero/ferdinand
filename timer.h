#include <stdint.h>

extern void startTimer(uint32_t start_time = 0);
extern void stopTimer();
extern void clearTimer();
extern uint32_t testTime();

extern char* hhmmss(char buf[9], uint32_t t);
extern char* mmss(char buf[6], uint32_t t);

