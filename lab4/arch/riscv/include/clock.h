#ifndef _CLOCK_H
#define _CLOCK_H

#define SBI_SETTIMER 0x0
#define TIMECLOCK 10000000

#include "clock.h"
#include "sbi.h"


unsigned long get_cycles();

void clock_set_next_event();

#endif
