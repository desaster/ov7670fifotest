#ifndef __UART_H
#define __UART_H

#include <inttypes.h>

void uart_init(void);
uint8_t uart_getc(uint8_t *c);
void uart_putc(uint8_t c);
void uart_puts(const char *str);

#endif

/* vim: set sw=4 et: */
