/*
 * Default ARMv7-M vector table used by the bare-metal (non-ThreadX) hello
 * apps. The four configurable-fault vectors (HardFault / MemManage /
 * BusFault / UsageFault) route to `_ARMV7M_Exception_default` — the naked
 * shim from //boards/common:fault that captures registers and prints a
 * decoded CFSR/HFSR/MMFAR/BFAR dump before halting. Everything else still
 * routes to `_hang` so an unwired interrupt is easy to spot.
 *
 * The reset entry points at `_reset+1` — the +1 sets the Thumb bit that
 * ARMv7-M requires on function pointers stored in the vector table
 * (see ARM DDI 0403 B1.5.2). Same convention for `_ARMV7M_Exception_default`.
 *
 * `_vectors` is exported; boards/common/reset.S references it when
 * programming VTOR.
 */

    .syntax unified
    .thumb

    .global _ARMV7M_Exception_default

    .section .vectors, "a", %progbits
    .align 2
    .global _vectors
_vectors:
    .word   __stack_top                       /* 0x00 initial SP */
    .word   _reset+1                          /* 0x04 Reset */
    .word   _hang+1                           /* 0x08 NMI */
    .word   _ARMV7M_Exception_default+1       /* 0x0C HardFault */
    .word   _ARMV7M_Exception_default+1       /* 0x10 MemManage */
    .word   _ARMV7M_Exception_default+1       /* 0x14 BusFault */
    .word   _ARMV7M_Exception_default+1       /* 0x18 UsageFault */
    .word   0
    .word   0
    .word   0
    .word   0
    .word   _hang+1                           /* 0x2C SVCall */
    .word   0
    .word   0
    .word   _hang+1                           /* 0x38 PendSV */
    .word   _hang+1                           /* 0x3C SysTick */
