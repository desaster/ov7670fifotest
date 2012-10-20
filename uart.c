/*
 * Original version from:
 * http://www.msp430launchpad.com/2010/08/half-duplex-software-uart-on-launchpad.html
 *
 * This version from: http://blog.hodgepig.org
 *
 * + Insignificant changes by me.
 */

#include <msp430.h>
#include <legacymsp430.h>

#include "uart.h"
#include "pins.h"

#define BAUDRATE 115200

#define BIT_TIME        (F_CPU / BAUDRATE)

/* more important than being a half bit is the fact that this affects the
 * whole timing of all the bits, so adjust accordingly. At higher speeds the
 * interrupt seems to be late anyways.. */
#define HALF_BIT_TIME   10

static volatile uint8_t bitCount; // Bit count, used when transmitting byte
static volatile unsigned int TXByte; // Value sent over UART when uart_putc() is called
static volatile unsigned int RXByte; // Value recieved once hasRecieved is set

static volatile uint8_t isReceiving = 0; // Status for when the device is receiving
static volatile uint8_t hasReceived = 0; // Lets the program know when a byte is received

void uart_init(void)
{
    P1SEL |= TXD;
    P1DIR |= TXD;

    P1DIR &= ~RXD;
    P1SEL &= ~RXD;

    P1IES |= RXD; // RXD Hi/lo edge interrupt
    P1IFG &= ~RXD; // Clear RXD (flag) before enabling interrupt
    P1IE |= RXD; // Enable RXD interrupt
}

uint8_t uart_getc(uint8_t *c)
{
    if (!hasReceived) {
        return 0;
    }
    *c = RXByte;
    hasReceived = 0;
    return 1;
}

void test(int a)
{
    return;
}

void uart_putc(uint8_t c)
{
    TXByte = c;

    while (isReceiving); // Wait for RX completion

    CCTL0 = OUT; // TXD Idle as Mark
    TACTL = TASSEL_2 + MC_2; // SMCLK, continuous mode

    bitCount = 0xA; // Load Bit counter, 8 bits + ST/SP
    CCR0 = TAR; // Initialize compare register

    CCR0 += BIT_TIME; // Set time till first bit
    TXByte |= 0x100; // Add stop bit to TXByte (which is logical 1)
    TXByte = TXByte << 1; // Add start bit (which is logical 0)

    CCTL0 = CCIS0 + OUTMOD0 + CCIE; // Set signal, intial value, enable interrupts

    while (CCTL0 & CCIE); // Wait for previous TX completion
}

void uart_puts(const char *str)
{
    while (*str != 0) {
        uart_putc(*str++);
    }
}


interrupt(PORT1_VECTOR) PORT1_ISR(void)
{
    isReceiving = 1;

    P1IE &= ~RXD; // Disable RXD interrupt
    P1IFG &= ~RXD; // Clear RXD IFG (interrupt flag)

    TACTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
    CCR0 = TAR; // Initialize compare register
    /* bit + half is a fix, is this okay? */
    CCR0 += BIT_TIME + HALF_BIT_TIME; // Set time till first bit
    //CCR0 = TAR + HALF_BIT_TIME;
    CCTL0 = OUTMOD1 + CCIE; // Disable TX and enable interrupts
    //P1OUT ^= BIT0;

    RXByte = 0; // Initialize RXByte
    bitCount = 9; // Load Bit counter, 8 bits + start bit
}

interrupt(TIMER0_A0_VECTOR) TIMERA0_ISR(void)
{
    if (!isReceiving)
    {
        CCR0 += BIT_TIME; // Add Offset to CCR0
        if (bitCount == 0) { // If all bits TXed
            TACTL = TASSEL_2; // SMCLK, timer off (for power consumption)
            CCTL0 &= ~ CCIE ; // Disable interrupt
        } else {
            CCTL0 |= OUTMOD2; // Set TX bit to 0
            if (TXByte & 0x01) {
                CCTL0 &= ~ OUTMOD2; // If it should be 1, set it to 1
            }
            TXByte = TXByte >> 1;
            bitCount --;
        }
    } else {
        CCR0 += BIT_TIME; // Add Offset to CCR0
        if (bitCount == 0) {
            TACTL = TASSEL_2; // SMCLK, timer off (for power consumption)
            CCTL0 &= ~ CCIE ; // Disable interrupt

            isReceiving = 0;

            P1IFG &= ~RXD; // clear RXD IFG (interrupt flag)
            P1IE |= RXD; // enabled RXD interrupt

            if ((RXByte & 0x201) == 0x200) { // Validate the start and stop bits are correct
                RXByte = RXByte >> 1; // Remove start bit
                RXByte &= 0xFF; // Remove stop bit
                hasReceived = 1;
            } else {
                int a = RXByte;
                if (a == 0) while (1);
                test(a);
            }
        } else {
            if ((P1IN & RXD) == RXD) { // If bit is set?
                RXByte |= 0x400; // Set the value in the RXByte
            }
            RXByte = RXByte >> 1; // Shift the bits down
            bitCount --;
        }
        //P1OUT ^= BIT0;
    }
}

/* vim: set sw=4 et: */
