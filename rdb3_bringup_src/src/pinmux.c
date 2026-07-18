#include "s32g3_regs.h"

/*
 * RDB3 UART0 USB port physical wiring
 *   FT232RQ USB-UART bridge -> pads PC_09 (TX) and PC_10 (RX)
 *   REF: reference_docs/S32G-VNP-RDB3_DesignFiles/Schematic/SPF-53060_F.pdf,
 *        page 26 ("UART" sheet).
 *    The SoC diagram shows these pads with the alt-function "PFE_UART_TX/RX"
 *    but this is only one of the several alt functions the pad carry.
 *
 * S32G3 pad numbering according to RM convention: PA_00..PA_15 = 0..15,
 *   PB_00..PB_15 = 16..31, so PC_09 = 32 + 9 = 41, PC_10 = 42.
 *
 *   This is confirmed by BootROM's serial-boot UART assignment
 *   REF: RM 31.12.4.1 Table 179 p1351 "RX = PAD[42], TX = PAD[41], LINFlexD_0 
 *        in UART mode."
 *
 * Alternate function values  taken from the S32G3_IOMUX.xlsx embedded in
 * the RM PDF, sheet "IO Signal Table":
 *   Row 316:  MSCR[41]  SSS = 0b001  ->  LIN0_TX (LINFlex_0)
 *   Row 326:  MSCR[42]  base pad entry (IBE only, no output alt used)
 *   Row 329:  IMCR[0]   SSS = 0b010  ->  LIN0_RX sourced from PC_10
 *
 * MSCR field diagram:
 * REF: RM 16.3.12 (p642).  IMCR is a an alternate function register.
 */
void pinmux_init(void)
{
    SIUL2_MSCR(41) = MSCR_OBE | MSCR_SSS(1);   /* PC_09 drives LIN0_TX */
    SIUL2_MSCR(42) = MSCR_IBE;                  /* PC_10 as input to pad */
    SIUL2_IMCR(0)  = MSCR_SSS(2);               /* LIN0_RX from PC_10    */
}
