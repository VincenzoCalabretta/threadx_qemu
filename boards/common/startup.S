/*
 * ARMv7-M reset stub, shared across all boards in this project.
 *
 * Both supported boards (QEMU mps2-an500 and S32G399A M7_0) are Cortex-M7
 * cores that start execution at the reset vector held in .vectors[1].
 * .vectors[0] is the initial SP. See ARMv7-M Architecture Reference Manual
 * ARM DDI 0403, section B1.5.2.
 *
 * The linker script (per board) is responsible for placing .vectors at the
 * address the hardware reads at reset:
 *   QEMU mps2-an500  .vectors -> 0x00000000 (SSRAM1 boot)
 *   S32G3 M7_0 SRAM  .vectors -> 0x34000000 (Internal SRAM), reached after the
 *                                 T32 load script writes VTOR
 *
 * VTOR (Vector Table Offset Register, ARM DDI 0403 B3.2.5) is written to
 * &_vectors so the code survives being relinked at a non-zero base — required
 * on S32G3, harmless on QEMU where the table sits at 0 already.
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

    /* VTOR = &_vectors */
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
