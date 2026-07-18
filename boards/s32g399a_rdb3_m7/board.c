#include "app/board.h"
#include "board_internal.h"

/*
 * Bring the S32G3 M7_0 to a state where LINFlexD_0 can transmit:
 *   clock_init  -- point LIN_BAUD_CLK at FIRC (48 MHz) so uart_init's baud
 *                  divisor math has a known input.
 *   pinmux_init -- route LIN0_TX/RX to the RDB3's FT232RQ USB-UART bridge pads.
 */
void board_init(void)
{
    clock_init();
    pinmux_init();
}
