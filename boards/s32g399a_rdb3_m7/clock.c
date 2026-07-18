#include "s32g3_regs.h"
#include "board_internal.h"

/*
 * Set MC_CGM_0 clock mux 8 LIN_BAUD_CLK feeder to FIRC = 48 MHz.
 *
 * Post-reset MUX_8 already selects FIRC
 * SELCTL reset value = 0,
 * REF: per RM- 25.5.6 p951,
 * but S32G3 BootROM re-programs this mux when it tries XOSC
 * REF: RM 31.12.4.2 p1351:
 * "BootROM uses XOSC as clock source for LIN_BAUD_CLK during boot.
 *  If XOSC fails to initialize, FIRC is selected
 *  as the MUX source.".
 *
 * Independently from state we halt in, we set the clock mux back to
 * FIRC explicitly so the LINFlexD baud calc in uart.c has a known input.
 *
 * Clock source encoding SELCTL = 0 -> FIRC_CLK from RM 24.3.1 Table 78 p884.
 *  SWIP poll is per RM 25.5.7 p954.
 */
void clock_init(void)
{
    MC_CGM_MUX8_CSC = MUX_CSC_SELCTL(0) | MUX_CSC_CLK_SW;
    while (MC_CGM_MUX8_CSS & MUX_CSS_SWIP) { }
}
