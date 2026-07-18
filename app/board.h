#ifndef APP_BOARD_H
#define APP_BOARD_H

/*
 * Interface each board port must implement. main.c calls these in order
 * to reach a working stdout, without knowing anything about which chip or
 * on-board peripheral is providing it.
 *
 *   board_init  clocks, pinmuxes, whatever the SoC needs before its UART
 *               can be brought up. No-op on boards where the reset state
 *               already lets the UART transmit (e.g. QEMU mps2-an500).
 *   uart_init   configure the UART for the board's fixed baud/framing.
 *   uart_puts   blocking write of a NUL-terminated string.
 */
void board_init(void);
void uart_init(void);
void uart_puts(const char *s);

#endif
