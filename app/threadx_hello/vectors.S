/*
 * Vector table for the ThreadX hello app. Same reset entry as the bare-metal
 * apps, but PendSV and SysTick point at the ThreadX port handlers so the
 * scheduler and timer tick work. SVCall/NMI/HardFault land on the ThreadX
 * "wedge in a tight loop" shims (see tx_initialize_low_level.S).
 *
 * `_reset+1` sets the Thumb bit that ARMv7-M requires on function pointers
 * stored in the vector table (ARM DDI 0403 B1.5.2).
 */

    .syntax unified
    .thumb

    .global _reset
    .global __tx_NMIHandler
    .global __tx_HardfaultHandler
    .global __tx_BadHandler
    .global __tx_SVCallHandler
    .global __tx_DBGHandler
    .global __tx_PendSVHandler
    .global __tx_SysTickHandler

    .section .vectors, "a", %progbits
    .align 2
    .global _vectors
_vectors:
    .word   __stack_top
    .word   _reset+1
    .word   __tx_NMIHandler
    .word   __tx_HardfaultHandler
    .word   __tx_BadHandler        /* MemManage */
    .word   __tx_BadHandler        /* BusFault */
    .word   __tx_BadHandler        /* UsageFault */
    .word   0
    .word   0
    .word   0
    .word   0
    .word   __tx_SVCallHandler
    .word   __tx_DBGHandler
    .word   0
    .word   __tx_PendSVHandler
    .word   __tx_SysTickHandler
