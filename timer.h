#include <stdint.h>

extern void startTimer();
extern void stopTimer();
extern void clearTimer();
extern uint32_t testTime();
extern char* timeString(char hhmmss[], uint32_t t);

