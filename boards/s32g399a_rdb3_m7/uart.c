#include "app/board.h"
#include "s32g3_regs.h"

/*
 * LINFlexD_0 in UART mode, 115200 8N1, buffer non-FIFO mode, polled TX.
 *
 * Init sequence per RM 49.4.4 p2474 "UART mode" and 49.5.6 p2503
 * UARTCR field descriptions.
 *
 * Baud calc: LDIV = f_LIN_BAUD_CLK / (16 * baud).  With clock_init()
 * setting MC_CGM_0 mux 8 to FIRC, f = 48 MHz:
 *   LDIV = 48_000_000 / (16 * 115200) = 26.0416
 *   LINIBRR = 26, LINFBRR = 1   ->  effective 115_108 baud
*
 * Also, RM 31.12.4.3 Table 180 p1351 lists 48 MHz (FIRC) ->
 * 115200 Bd as a supported baud/clock pair for LINFlexD_0.
 *
 * uart_putc uses buffer mode, write byte to BDRL, poll UARTSR.DTFTFF
 * then W1C the flag.  DTFTFF is W1C in buffer mode but read-only in FIFO mode
 * RM 49.5.7 p2508.
 */
void uart_init(void)
{
    LINCR1  = LINCR1_RESERVED_1 | LINCR1_INIT;
    UARTCR  = UARTCR_UART;
    UARTCR  = UARTCR_UART | UARTCR_WL0
            | UARTCR_TxEn | UARTCR_RxEn;
    LINIBRR = 26;
    LINFBRR = 1;
    LINCR1  = LINCR1_RESERVED_1;
}

static void uart_putc(char c)
{
    BDRL = (uint32_t)(uint8_t)c;
    while ((UARTSR & UARTSR_DTFTFF) == 0) { }
    UARTSR = UARTSR_DTFTFF;
}

void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}
