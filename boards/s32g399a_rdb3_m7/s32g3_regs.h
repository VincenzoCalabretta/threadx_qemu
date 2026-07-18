#ifndef BOARDS_S32G399A_RDB3_M7_S32G3_REGS_H
#define BOARDS_S32G399A_RDB3_M7_S32G3_REGS_H

/*
 * All addresses and bit fields on this page are taken from RM attachment
 * S32G3_memory_map.xlsx. (Revision 5).
 *
 * "RM x.y.z (pN)" citations use the section number and page number.
 */

#include <stdint.h>

#define REG32(a) (*(volatile uint32_t *)(a))

/*
 * SIUL2_0 System Integration Unit Lite 2, Chip Configuration bus
 *   Base:            0x4009_C000       RM 16.3.1 (p603)
 *   MSCRa offset:    0x240 + a * 4     RM 16.3.12 (p641)
 *   IMCRb offset:    0xA40 + b * 4     RM 16.3.13 (p645)
 */
#define SIUL2_0_BASE       0x4009C000u
#define SIUL2_MSCR(n)      REG32(SIUL2_0_BASE + 0x240u + (n) * 4u)
#define SIUL2_IMCR(n)      REG32(SIUL2_0_BASE + 0xA40u + (n) * 4u)

/* MSCR field diagram RM 16.3.12 (p642) */
#define MSCR_OBE           (1u << 21)         /* Output Buffer Enable */
#define MSCR_IBE           (1u << 19)         /* Input  Buffer Enable */
#define MSCR_SSS(x)        ((x) & 0x7u)       /* Source Signal Select, per IOMUX xlsx */

/*
 * MC_CGM_0 Clock Generation Module 0
 *   Base:            0x4003_0000       RM 25.5.1 (p946)
 *   Mux 8 feeds LIN_BAUD_CLK           RM 24.3.2.1 Table 79 (p887)
 *                                      RM 24.7.2.9 Figure 102 (p927)
 *   MUX_8_CSC offset 0x500             RM 25.5.1 memory map (p947)
 *   MUX_8_CSS offset 0x504             RM 25.5.1 memory map (p947)
 *   SELCTL clock-source encoding       RM 24.3.1 Table 78 (p884): 0 = FIRC_CLK
 */
#define MC_CGM_0_BASE      0x40030000u
#define MC_CGM_MUX8_CSC    REG32(MC_CGM_0_BASE + 0x500u)
#define MC_CGM_MUX8_CSS    REG32(MC_CGM_0_BASE + 0x504u)

/* CSC/CSS field layout RM 25.5.6 (p951) and 25.5.7 (p953) */
#define MUX_CSC_CLK_SW     (1u << 2)          /* trigger clock switch */
#define MUX_CSC_SELCTL(x)  (((x) & 0xFu) << 24)
#define MUX_CSS_SWIP       (1u << 16)         /* switch in progress */
#define MUX_CSS_SELSTAT(v) (((v) >> 24) & 0xFu)

/*
 * LINFlexD_0 LIN/UART controller
 *   Base:            0x401C_8000       RM 49.5.1 (p2490)
 *   Register offsets                   RM Table (p2491)
 */
#define LINFLEXD0_BASE     0x401C8000u
#define LINCR1             REG32(LINFLEXD0_BASE + 0x00u)   /* RM 49.5.2 (p2491) */
#define UARTCR             REG32(LINFLEXD0_BASE + 0x10u)   /* RM 49.5.6 (p2502) */
#define UARTSR             REG32(LINFLEXD0_BASE + 0x14u)   /* RM 49.5.7 (p2508) */
#define LINFBRR            REG32(LINFLEXD0_BASE + 0x24u)   /* RM 49.5.11 (p2514) */
#define LINIBRR            REG32(LINFLEXD0_BASE + 0x28u)   /* RM 49.5.12 (p2515) */
#define BDRL               REG32(LINFLEXD0_BASE + 0x38u)   /* RM Table (p2491) */

/* LINCR1 field diagram RM 49.5.2 (p2492).
 * Reset value 0x0082: bit 7 is a reserved bit whose reset value is 1 and the
 * RM notes "each reserved bit must be written to its original reset value". */
#define LINCR1_INIT        (1u << 0)          /* Initialization Mode Request */
#define LINCR1_SLEEP       (1u << 1)          /* Sleep Mode Request           */
#define LINCR1_RESERVED_1  (1u << 7)          /* reset = 1, must preserve     */

/* UARTCR field diagram RM 49.5.6 (p2503).
 * WL1:WL0 = 01 with PCE=0 selects 8 data bits, no parity (Table p2506). */
#define UARTCR_UART        (1u << 0)          /* UART mode (else LIN)         */
#define UARTCR_WL0         (1u << 1)          /* Word Length bit 0            */
#define UARTCR_TxEn        (1u << 4)          /* Transmitter enable           */
#define UARTCR_RxEn        (1u << 5)          /* Receiver enable              */

/* UARTSR RM 49.5.7 p2510. DTFTFF is W1C in buffer mode. */
#define UARTSR_DTFTFF      (1u << 1)          /* Data Transmission Completed  */

#endif
