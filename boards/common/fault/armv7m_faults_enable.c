/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @brief Enable the ARMv7-M configurable faults (MemManage, BusFault,
 *   UsageFault) so each fires on its own vector instead of always
 *   escalating to HardFault.
 *
 * Called from _reset in boards/common/reset.S before bl main. All four
 * fault vector slots still route to _ARMV7M_Exception_default — the only
 * observable change is that BFSR/MMFSR/UFSR conditions no longer set
 * HFSR.FORCED, and CPU_Exception_frame.vector reports the specific fault
 * (MemManage=4, BusFault=5, UsageFault=6) rather than HardFault=3.
 */

#include <rtems/score/armv7m.h>

void _ARMV7M_Enable_configurable_faults(void)
{
  _ARMV7M_SCB->shcsr |=
      ARMV7M_SCB_SHCSR_MEMFAULTENA
    | ARMV7M_SCB_SHCSR_BUSFAULTENA
    | ARMV7M_SCB_SHCSR_USGFAULTENA;
}
