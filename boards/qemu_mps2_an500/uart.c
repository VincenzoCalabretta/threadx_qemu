#include "app/board.h"
#include "uart_regs.h"
#include <rtems/bspIo.h>

void uart_init(void)
{
    /* QEMU ignores the divisor value; still write it so the driver looks sane. */
    UART_BAUD = 16u;
    UART_CTRL = UART_CTRL_TXEN;
}

static void uart_putc(char c)
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

/* Bind //boards/common:printk's char sink to this board's polled TX. */
BSP_output_char_function_type      BSP_output_char = uart_putc;
BSP_polling_getchar_function_type  BSP_poll_char   = 0;
