/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Local rewrite of RTEMS's _ARM_Exception_default (which upstream tail-calls
 * rtems_fatal). We are not linking against the RTEMS fatal machinery, so
 * this print-and-halt shim is what the naked-asm entry in
 * armv7m_exception_default.c branches to.
 */

#include <rtems/bspIo.h>
#include <rtems/score/cpu.h>

__attribute__((__noreturn__))
void _ARM_Exception_default(CPU_Exception_frame *frame)
{
  _CPU_Exception_frame_print(frame);
  /* WFI rather than BKPT here: BKPT with the debug system disabled
   * escalates to HardFault, and we are already at HardFault priority —
   * QEMU treats the re-escalation as a lockup and crashes the emulator
   * before the user sees the register dump on subsequent faults. WFI
   * simply parks the core; a real debugger can still halt via DHCSR. */
  for (;;) {
    __asm__ volatile ("wfi");
  }
}
