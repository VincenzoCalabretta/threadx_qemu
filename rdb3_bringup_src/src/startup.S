/*
 * Minimal Cortex-M7 startup for S32G3 M7_0, executing from Internal SRAM
 * linked at 0x34000000.
 *
 * Vector-table layout is defined by the
 * ARMv7-M Architecture Reference Manual ARM DDI 0403, section B1.5.2
 *
 * VTOR register at 0xE000_ED08 is the ARMv7-M Vector Table Offset
 * Register ARM DDI 0403 B3.2.5. 
 * On reset it points into the ROM vector table;
 * The script redirects it to the SRAM copy after setting SP.
 *
 * SRAM base 0x34000000 is the Internal SRAM window per S32G3
 * memory-map attachment S32G3_memory_map.xlsx sheet
 * "Overview Memory Map 40bit" row 51 (20 MB region 0x34000000-0x353FFFFF).
 * This is the same base the Lauterbach S32G3 M7 demo uses 
 * (see ~/t32/demo/arm/hardware/s32g3/misc/s32g3_enable_m7_0.cmm and in the
 * demo makefile RAMSTART=0x34000000).
 */

    .syntax unified
    .thumb

    .section .vectors, "a", %progbits
    .align 2
    .global _vectors
_vectors:
    .word   __stack_top          /* 0x00 initial SP */
    .word   _reset+1             /* 0x04 Reset */
    .word   _hang+1              /* 0x08 NMI */
    .word   _hang+1              /* 0x0C HardFault */
    .word   _hang+1              /* 0x10 MemManage */
    .word   _hang+1              /* 0x14 BusFault */
    .word   _hang+1              /* 0x18 UsageFault */
    .word   0
    .word   0
    .word   0
    .word   0
    .word   _hang+1              /* 0x2C SVCall */
    .word   0
    .word   0
    .word   _hang+1              /* 0x38 PendSV */
    .word   _hang+1              /* 0x3C SysTick */

    .section .text._reset, "ax", %progbits
    .thumb_func
    .global _reset
_reset:
    ldr     r0, =__stack_top
    mov     sp, r0

    /* Point VTOR at our vector table (ARM DDI 0403 B3.2.5). */
    ldr     r0, =0xE000ED08
    ldr     r1, =_vectors
    str     r1, [r0]

    /* copy .data from LMA to VMA */
    ldr     r0, =__data_lma
    ldr     r1, =__data_start
    ldr     r2, =__data_end
1:  cmp     r1, r2
    beq     2f
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       1b
2:

    /* zero .bss */
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    movs    r2, #0
3:  cmp     r0, r1
    beq     4f
    str     r2, [r0], #4
    b       3b
4:

    bl      main
    b       _hang

    .thumb_func
    .global _hang
_hang:
    b       _hang
