/*
 * ARMv7-M reset stub, shared across every app in this project.
 *
 * The board's linker script contains ENTRY(_reset), which the linker treats
 * as `-u _reset` — that alone pulls this object out of the archive, so no
 * alwayslink is required.
 *
 * VTOR (Vector Table Offset Register, ARM DDI 0403 B3.2.5) is written to
 * &_vectors so the code survives being relinked at a non-zero base — required
 * on S32G3, harmless on QEMU where the table sits at 0 already. Whatever
 * .S file provides the `.vectors` section is responsible for exporting the
 * `_vectors` symbol.
 */

    .syntax unified
    .thumb

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

    /* Enable MemManage / BusFault / UsageFault so they fire on their own
     * vectors instead of always escalating to HardFault with HFSR.FORCED.
     * All four vector slots still route to _ARMV7M_Exception_default (see
     * vectors_bare.S); this only changes CPU_Exception_frame.vector and
     * CFSR classification. */
    bl      _ARMV7M_Enable_configurable_faults

    bl      main
    b       _hang

    .thumb_func
    .global _hang
_hang:
    b       _hang
