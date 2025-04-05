#ifndef PIR_H
#define PIR_H

#include "driver/gpio.h"

#ifdef USING_MAIN_PCB
#define PIR_GPIO 13
#else
#define PIR_GPIO 23
#endif

void sensor_init();
#endif // PIR_H