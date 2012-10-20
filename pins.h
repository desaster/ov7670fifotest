#ifndef __PINS_H
#define __PINS_H

#include <msp430.h>

#define VSYNC   BIT0 // P1.0
#define TXD     BIT1 // P1.1
#define RXD     BIT2 // P1.2
#define RRST    BIT3 // P1.3 (note: on launchpad P1.3 has a debouncing cap)
#define RCLK    BIT4 // P1.4
#define WEN     BIT5 // P1.5
#define SCL     BIT6 // P1.6
#define SDA     BIT7 // P1.7

#endif

/* vim: set sw=4 et: */
