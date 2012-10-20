#include <msp430g2452.h>
#include <msp430.h>
#include <legacymsp430.h>

#include <inttypes.h>

#include "i2c.h"
#include "ov7670.h"
#include "tprintf.h"
#include "pins.h"
#include "utils.h"

void ov7670_set(uint8_t addr, uint8_t val)
{
    i2c_start();
    i2c_write8(OV7670_ADDR);
    i2c_write8(addr);
    i2c_write8(val);
    i2c_stop();
}

uint8_t ov7670_get(uint8_t addr)
{
    uint8_t retval;

    i2c_start();
    i2c_write8(OV7670_ADDR);
    i2c_write8(addr);
    i2c_stop();

    __delay_cycles(16 * 1000);

    i2c_start();
    i2c_write8(OV7670_ADDR + 1);
    retval = i2c_read8(0xFF);
    i2c_stop();

    return retval;
}

uint8_t ov7670_init(void)
{
    if (ov7670_get(REG_PID) != 0x76) {
        return 0;
    }

    ov7670_set(REG_COM7, 0x80); /* reset to default values */
    ov7670_set(REG_CLKRC, 0x80);
    ov7670_set(REG_COM11, 0x0A);
    ov7670_set(REG_TSLB, 0x04);
    ov7670_set(REG_TSLB, 0x04);
    ov7670_set(REG_COM7, 0x04); /* output format: rgb */

    ov7670_set(REG_RGB444, 0x00); /* disable RGB444 */
    ov7670_set(REG_COM15, 0xD0); /* set RGB565 */

    /* not even sure what all these do, gonna check the oscilloscope and go
     * from there... */
    ov7670_set(REG_HSTART, 0x16);
    ov7670_set(REG_HSTOP, 0x04);
    ov7670_set(REG_HREF, 0x24);
    ov7670_set(REG_VSTART, 0x02);
    ov7670_set(REG_VSTOP, 0x7a);
    ov7670_set(REG_VREF, 0x0a);
    ov7670_set(REG_COM10, 0x02);
    ov7670_set(REG_COM3, 0x04);
    ov7670_set(REG_MVFP, 0x3f);

    /* 160x120, i think */
    //ov7670_set(REG_COM14, 0x1a); // divide by 4
    //ov7670_set(0x72, 0x22); // downsample by 4
    //ov7670_set(0x73, 0xf2); // divide by 4

    /* 320x240: */
    ov7670_set(REG_COM14, 0x19);
    ov7670_set(0x72, 0x11);
    ov7670_set(0x73, 0xf1);

    // test pattern
    //ov7670_set(0x70, 0xf0);
    //ov7670_set(0x71, 0xf0);

    // COLOR SETTING
    ov7670_set(0x4f, 0x80);
    ov7670_set(0x50, 0x80);
    ov7670_set(0x51, 0x00);
    ov7670_set(0x52, 0x22);
    ov7670_set(0x53, 0x5e);
    ov7670_set(0x54, 0x80);
    ov7670_set(0x56, 0x40);
    ov7670_set(0x58, 0x9e);
    ov7670_set(0x59, 0x88);
    ov7670_set(0x5a, 0x88);
    ov7670_set(0x5b, 0x44);
    ov7670_set(0x5c, 0x67);
    ov7670_set(0x5d, 0x49);
    ov7670_set(0x5e, 0x0e);
    ov7670_set(0x69, 0x00);
    ov7670_set(0x6a, 0x40);
    ov7670_set(0x6b, 0x0a);
    ov7670_set(0x6c, 0x0a);
    ov7670_set(0x6d, 0x55);
    ov7670_set(0x6e, 0x11);
    ov7670_set(0x6f, 0x9f);

    ov7670_set(0xb0, 0x84);

    return 1;
}

void ov7670_capture(void)
{
    /* Capture frame */
    while ((P1IN & VSYNC));     // wait for an old frame to end
    while (!(P1IN & VSYNC));    // wait for a new frame to start
    P1OUT |= WEN;               // enable writing to fifo
    while ((P1IN & VSYNC));     // wait for the current frame to end
    P1OUT &= ~WEN;              // disable writing to fifo
    wait();
    P1OUT |= RRST;
}

/* Reset the al422 read pointer */
void ov7670_rrst(void)
{
    /* Since RRST (P1.3) has a debouncing cap, let's go really slowly
     * here... */
    P1OUT &= ~RRST;
    __delay_cycles(500);
    P1OUT |= RCLK;
    __delay_cycles(100);
    P1OUT |= RRST;
    P1OUT &= ~RCLK;
}

/* Read one byte from the al422 */
uint8_t ov7670_read(void)
{
    uint8_t val;

    P1OUT |= RCLK;
    val = P2IN;
    //__delay_cycles(10);
    P1OUT &= ~RCLK;
    //__delay_cycles(10);

    return val;
}

/* vim: set sw=4 et: */
