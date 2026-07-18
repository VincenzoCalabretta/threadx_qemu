#include "app/board.h"

/*
 * QEMU's mps2-an500 comes out of reset with the CMSDK UART already reachable —
 * no clock tree or pinmux to program before uart_init(). Provide an empty
 * board_init() so main.c can stay board-agnostic.
 */
void board_init(void) { }
