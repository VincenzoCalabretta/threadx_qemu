#ifndef UART_H
#define UART_H

#include <stdint.h>

/*
 * CMSDK UART0 as exposed by QEMU's `mps2-an500` machine.
 * Register map from ARM DDI 0479C, "Cortex-M System Design Kit TRM", chapter 3.
 * QEMU model: hw/char/cmsdk-apb-uart.c.
 *
 * With `qemu-system-arm -nographic`, UART0's TX byte stream is forwarded to
 * host stdout so we don't need semihosting or a debug adapter.
 */
#define UART0_BASE 0x40004000UL

#define UART_DATA  (*(volatile uint32_t *)(UART0_BASE + 0x00u))
#define UART_STATE (*(volatile uint32_t *)(UART0_BASE + 0x04u))
#define UART_CTRL  (*(volatile uint32_t *)(UART0_BASE + 0x08u))
#define UART_BAUD  (*(volatile uint32_t *)(UART0_BASE + 0x10u))

#define UART_STATE_TXFULL (1u << 0)
#define UART_CTRL_TXEN    (1u << 0)

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);

#endif
