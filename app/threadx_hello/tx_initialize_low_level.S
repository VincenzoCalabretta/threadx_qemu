/*
 * ThreadX low-level init for our Cortex-M7 apps. Adapted from
 * threadx/ports/cortex_m7/gnu/example_build/tx_initialize_low_level.S:
 *
 *   - `__RAM_segment_used_end__` → `__bss_end` so we can reuse the linker
 *     symbols already defined in the per-board linker scripts.
 *   - SYSTICK_CYCLES fixed to a hard-coded reload; SysTick accuracy is not
 *     load-bearing for a hello world (tx_thread_sleep just needs *some*
 *     tick), so we avoid propagating per-board clock configuration here.
 *
 * `_vectors` is provided by vectors.S in this same package.
 */

    .global     _tx_thread_system_stack_ptr
    .global     _tx_initialize_unused_memory
    .global     __bss_end
    .global     _tx_timer_interrupt
    .global     _vectors

SYSTICK_CYCLES = 24000 - 1

    .text 32
    .align 4
    .syntax unified

    .global  _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:

    CPSID   i

    /* First free RAM address (unused memory pool for tx_application_define). */
    LDR     r0, =_tx_initialize_unused_memory
    LDR     r1, =__bss_end
    ADD     r1, r1, #4
    STR     r1, [r0]

    /* VTOR = &_vectors (redundant with reset.S but matches upstream port). */
    MOV     r0, #0xE000E000
    LDR     r1, =_vectors
    STR     r1, [r0, #0xD08]

    /* System stack pointer = *(uint32_t*)_vectors (the initial SP slot). */
    LDR     r0, =_tx_thread_system_stack_ptr
    LDR     r1, =_vectors
    LDR     r1, [r1]
    STR     r1, [r0]

    /* Enable DWT cycle counter — cheap, and some ThreadX modules read it. */
    LDR     r0, =0xE0001000
    LDR     r1, [r0]
    ORR     r1, r1, #1
    STR     r1, [r0]

    /* SysTick reload + enable (processor clock, tick int, counter on). */
    MOV     r0, #0xE000E000
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]
    MOV     r1, #0x7
    STR     r1, [r0, #0x10]

    /* System handler priorities. SVCall and PendSV must be lowest. */
    LDR     r1, =0x00000000
    STR     r1, [r0, #0xD18]
    LDR     r1, =0xFF000000
    STR     r1, [r0, #0xD1C]
    LDR     r1, =0x40FF0000
    STR     r1, [r0, #0xD20]

    BX      lr


/* Fault / unused-vector shims used by vectors.S. */
    .global  __tx_BadHandler
    .thumb_func
__tx_BadHandler:
    B       __tx_BadHandler

    .global  __tx_HardfaultHandler
    .thumb_func
__tx_HardfaultHandler:
    B       __tx_HardfaultHandler

    .global  __tx_SVCallHandler
    .thumb_func
__tx_SVCallHandler:
    B       __tx_SVCallHandler

    .global  __tx_NMIHandler
    .thumb_func
__tx_NMIHandler:
    B       __tx_NMIHandler

    .global  __tx_DBGHandler
    .thumb_func
__tx_DBGHandler:
    B       __tx_DBGHandler


/* SysTick trampoline into ThreadX's timer interrupt. */
    .global  __tx_SysTickHandler
    .thumb_func
__tx_SysTickHandler:
    PUSH    {r0, lr}
    BL      _tx_timer_interrupt
    POP     {r0, lr}
    BX      lr
