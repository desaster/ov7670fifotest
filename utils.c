#include <msp430g2452.h>
#include <msp430.h>
#include <legacymsp430.h>

#include "utils.h"

void wait(void)
{
    volatile int i;
    for (i = 0; i < 100; i ++) {
        __delay_cycles(16 * 1000);
    }
}

/* vim: set sw=4 et: */
