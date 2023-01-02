#include <stdint.h>

extern void startTimer();
extern void stopTimer();
extern void clearTimer();
extern uint32_t testTime();
extern char* hhmmss(char buf[9], uint32_t t);
extern char* mmss(char buf[6], uint32_t t);

