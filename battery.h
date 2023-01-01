#include <stdint.h>

// return battery voltage in 10mV units
extern unsigned int battery();

// convert battery voltage to fixed-length string
extern char *vString(char vstr[7], unsigned int vbat);
    
