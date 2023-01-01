#include <stdint.h>
#include <Arduino.h>
#include "hardware.h"
#include "battery.h"

#define VCC     4.96
#define R5      1000.
#define R6      510.      

// return battery voltage in 10mV units
unsigned int battery()
{
    const float scale = VCC * (R5 + R6) / (1024. * R6);
    int ain = analogRead(BATTERY);
    return (unsigned int)(ain * scale * 100.0 + 0.5);
}

// convert battery voltage to fixed-length string
char *vString(char vstr[], unsigned int vbat)
{
    vstr[0] = (vbat >= 1000) ? '1' : ' ';
    vstr[1] = '0' + (vbat / 100) % 10;
    vstr[2] = '.';
    vstr[3] = '0' + (vbat / 10) % 10;
    vstr[4] = '0' + vbat % 10;
    vstr[5] = 'V';
    vstr[6] = '\0';
    return vstr;
}
    
