/*
 * Minimal ARMv7-M startup for QEMU mps2-an500.
 *
 * QEMU's `-kernel` loads the ELF according to its program headers and starts
 * execution from the reset vector at .vectors[1]. .vectors is placed at
 * address 0x00000000 by the linker script.
 *
 * Vector-table layout: ARMv7-M ARM (DDI 0403) section B1.5.2.
 * VTOR write is not strictly needed since the table sits at 0 already, but
 * we do it anyway so the code survives being relinked at a non-zero base.
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
