#include "uart.h"

void uart_init(void)
{
    /* QEMU ignores the divisor value; still write it so the driver looks sane. */
    UART_BAUD = 16u;
    UART_CTRL = UART_CTRL_TXEN;
}

void uart_putc(char c)
{
    while (UART_STATE & UART_STATE_TXFULL) { }
    UART_DATA = (uint32_t)(unsigned char)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}
