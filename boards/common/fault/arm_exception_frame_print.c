/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @brief _CPU_Exception_frame_print for ARMv7-M: dump the general-purpose
 *   register file the naked-asm shim captured, then decode CFSR / HFSR
 *   (MMFSR + BFSR + UFSR + hard-fault causes) with MMFAR / BFAR when their
 *   *VALID bits are set.
 */

/*
 * Vendored from RTEMS cpukit/score/cpu/arm/arm-exception-frame-print.c.
 * Trimmed: dropped the ARM_MULTILIB_VFP register dump (this project does
 * not save VFP state) and the ARM_MULTILIB_ARCH_V4 alternative in the
 * general-purpose-register printout (we always build for ARMv7-M).
 *
 * Copyright (C) 2012, 2013 embedded brains GmbH & Co. KG
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <inttypes.h>

#include <rtems/score/cpu.h>
#include <rtems/score/armv7m.h>
#include <rtems/bspIo.h>

static void _ARM_Cortex_M_fault_info_print( void )
{
  /*
   * prints content of additional debugging registers
   * available on Cortex-Mx where x > 0 cores.
   */
  uint32_t cfsr = _ARMV7M_SCB->cfsr;
  uint8_t mmfsr = ARMV7M_SCB_CFSR_MMFSR_GET( cfsr );
  uint8_t bfsr = ( ARMV7M_SCB_CFSR_BFSR_GET( cfsr ) >> 8 );
  uint16_t ufsr = ( ARMV7M_SCB_CFSR_UFSR_GET( cfsr ) >> 16 );
  uint32_t hfsr = _ARMV7M_SCB->hfsr;
  if ( mmfsr > 0 ) {
    printk( "MMFSR= 0x%08" PRIx32 " (memory fault)\n", (uint32_t) mmfsr );
    if ( ( mmfsr & 0x1 ) != 0 ) {
      printk( "  IACCVIOL   : 1  (instruction access violation)\n" );
    }
    if ( ( mmfsr & 0x2 ) != 0 ) {
      printk( "  DACCVIOL   : 1  (data access violation)\n" );
    }
    if ( (mmfsr & 0x8 ) != 0 ) {
      printk(
        "  MUNSTKERR  : 1  (fault on unstacking on exception return)\n"
      );
    }
    if ( ( mmfsr & 0x10 ) != 0 ) {
      printk( "  MSTKERR    : 1  (fault on stacking on exception entry)\n" );
    }
    if ( (mmfsr & 0x20 ) != 0 ) {
      printk( "  MLSPERR    : 1  (fault during lazy FP stack preservation)\n" );
    }
    if ( (mmfsr & 0x80 ) != 0 ) {
      printk(
        "  MMFARVALID : 1 -> 0x%08" PRIx32 " (error address)\n",
        _ARMV7M_SCB->mmfar
      );
    }
    else {
      printk( "  MMFARVALID : 0  (undetermined error address)\n" );
    }
  }
  if ( bfsr > 0 ) {
    printk( "BFSR = 0x%08" PRIx32 " (bus fault)\n", (uint32_t) bfsr );
    if ( ( bfsr & 0x1 ) != 0 ) {
      printk( "  IBUSERR    : 1  (instruction fetch error)\n" );
    }
    if ( (bfsr & 0x2 ) != 0 ) {
      printk(
        "  PRECISERR  : 1  (data bus error with known exact location)\n"
      );
    }
    if ( ( bfsr & 0x4) != 0 ) {
      printk(
        "  IMPRECISERR: 1  (data bus error without known exact location)\n"
      );
    }
    if ( (bfsr & 0x8 ) != 0 ) {
      printk(
        "  UNSTKERR   : 1  (fault on unstacking on exception return)\n"
      );
    }
    if ( ( bfsr & 0x10 ) != 0 ) {
      printk( "  STKERR     : 1  (fault on stacking on exception entry)\n" );
    }
    if ( ( bfsr & 0x20 ) != 0 ) {
      printk( "  LSPERR     : 1  (fault during lazy FP stack preservation)\n" );
    }
    if ( (bfsr & 0x80 ) != 0 ) {
      printk(
        "  BFARVALID  : 1 -> 0x%08" PRIx32 "  (error address)\n",
        _ARMV7M_SCB->bfar
      );
    }
    else {
      printk( "  BFARVALID  : 0  (undetermined error address)\n" );
    }
  }
  if ( ufsr > 0 ) {
    printk( "UFSR = 0x%08" PRIx32 " (usage fault)\n", (uint32_t) ufsr);
    if ( (ufsr & 0x1 ) != 0 ) {
      printk( "  UNDEFINSTR : 1  (undefined instruction issued)\n");
    }
    if ( (ufsr & 0x2 ) != 0 ) {
      printk(
        "  INVSTATE   : 1"
        "  (invalid instruction state"
        " (Thumb not set in EPSR or invalid IT state in EPSR))\n"
      );
    }
    if ( (ufsr & 0x4 ) != 0 ) {
      printk( "  INVPC      : 1  (integrity check failure on EXC_RETURN)\n" );
    }
    if ( (ufsr & 0x8 ) != 0 ) {
      printk(
        "  NOCP       : 1"
        "  (coprocessor instruction issued"
        " but coprocessor disabled or non existent)\n"
      );
    }
    if ( ( ufsr & 0x100) != 0 ) {
      printk( "  UNALIGNED  : 1  (unaligned access operation occurred)\n" );
    }
    if ( ( ufsr & 0x200) != 0 ) {
      printk( "  DIVBYZERO  : 1  (division by zero)" );
    }
  }
  if ( (hfsr & (
    ARMV7M_SCB_HFSR_VECTTBL_MASK
    | ARMV7M_SCB_HFSR_DEBUGEVT_MASK
    | ARMV7M_SCB_HFSR_FORCED_MASK
    ) ) != 0 ) {
    printk( "HFSR = 0x%08" PRIx32 " (hard fault)\n", hfsr );
    if ( (hfsr & ARMV7M_SCB_HFSR_VECTTBL_MASK ) != 0 ) {
      printk(
        "  VECTTBL    : 1  (error in address located in vector table)\n"
      );
    }
    if ( (hfsr & ARMV7M_SCB_HFSR_FORCED_MASK ) != 0 ) {
      printk(
        "  FORCED     : 1  (configurable fault escalated to hard fault)\n"
      );
    }
    if ( (hfsr & ARMV7M_SCB_HFSR_DEBUGEVT_MASK ) != 0 ) {
      printk(
        "  DEBUGEVT   : 1  (debug event occurred with debug system disabled)\n"
      );
    }
  }
}

void _CPU_Exception_frame_print( const CPU_Exception_frame *frame )
{
  printk(
    "\n"
    "R0   = 0x%08" PRIx32 " R8  = 0x%08" PRIx32 "\n"
    "R1   = 0x%08" PRIx32 " R9  = 0x%08" PRIx32 "\n"
    "R2   = 0x%08" PRIx32 " R10 = 0x%08" PRIx32 "\n"
    "R3   = 0x%08" PRIx32 " R11 = 0x%08" PRIx32 "\n"
    "R4   = 0x%08" PRIx32 " R12 = 0x%08" PRIx32 "\n"
    "R5   = 0x%08" PRIx32 " SP  = 0x%08" PRIx32 "\n"
    "R6   = 0x%08" PRIx32 " LR  = 0x%08" PRIxPTR "\n"
    "R7   = 0x%08" PRIx32 " PC  = 0x%08" PRIxPTR "\n"
    "XPSR = 0x%08" PRIx32 " "
    "VEC = 0x%08" PRIxPTR "\n",
    frame->register_r0,
    frame->register_r8,
    frame->register_r1,
    frame->register_r9,
    frame->register_r2,
    frame->register_r10,
    frame->register_r3,
    frame->register_r11,
    frame->register_r4,
    frame->register_r12,
    frame->register_r5,
    frame->register_sp,
    frame->register_r6,
    (intptr_t) frame->register_lr,
    frame->register_r7,
    (intptr_t) frame->register_pc,
    frame->register_xpsr,
    (intptr_t) frame->vector
  );

  _ARM_Cortex_M_fault_info_print();
}
