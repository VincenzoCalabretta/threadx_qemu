/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @brief Naked-asm shim installed in the HardFault / MemManage / BusFault /
 *   UsageFault vector slots. Grabs the caller's register file, chooses
 *   MSP vs PSP based on EXC_RETURN's SPSEL bit, undoes the CPU-stacked
 *   frame (including the 4-byte SP alignment when the stacker set it),
 *   materialises a CPU_Exception_frame on the current stack, and tail-
 *   calls _ARM_Exception_default with it.
 */

/*
 * Vendored from RTEMS cpukit/score/cpu/arm/armv7m-exception-default.c.
 * Trimmed: the ARM_MULTILIB_VFP path is dropped (this project does not
 * enable FPU context save; vfp_context is always stored as NULL). The
 * upstream ARM_MULTILIB_ARCH_V7M ifdef wrapper is also dropped because
 * this file only ever builds against v7m in our tree. No other logic
 * changes.
 *
 * Copyright (c) 2013 embedded brains GmbH & Co. KG
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

#include <stddef.h>

#include <rtems/score/armv7m.h>
#include <rtems/score/cpu.h>

void __attribute__((naked)) _ARMV7M_Exception_default(void)
{
  __asm__ volatile (
    "sub sp, %[cpufsz]\n"       /* Allocate space for a CPU_Exception_frame. */
    "stm sp, {r0-r12}\n"
    "tst lr, #4\n"              /* EXC_RETURN.SPSEL: 0 = MSP, 1 = PSP */
    "itte eq\n"
    "mrseq r3, msp\n"           /*   then: we were using MSP */
    "addeq r3, %[cpufsz]\n"     /*         undo our own sub sp to recover old MSP */
    "mrsne r3, psp\n"           /*   else: we were using PSP */
    "add r2, r3, %[v7mlroff]\n"
    "add r1, sp, %[cpuspoff]\n"
    "ldm r2, {r4-r6}\n"         /* r4=LR, r5=PC, r6=xPSR from HW frame */
    "add r3, #32\n"             /* Skip the 8-word HW frame (no FP save). */
    "tst r6, #512\n"            /* xPSR.SPREALIGN (bit 9) — 4-byte alignment */
    "it ne\n"
    "addne r3, #4\n"            /*   then: undo the alignment padding */
    "stm r1, {r3-r6}\n"         /* SP/LR/PC/xPSR into CPU_Exception_frame */
    "mrs r1, ipsr\n"
    "str r1, [sp, %[cpuvecoff]]\n"

    "mov r0, sp\n"              /* Argument for _ARM_Exception_default */

    /* vfp_context = NULL (no VFP save in this build) */
    "add r3, sp, %[cpuvfpoff]\n"
    "mov r1, #0\n"
    "str r1, [r3]\n"

    "b _ARM_Exception_default\n"
    :
    : [cpufsz]    "i" (sizeof(CPU_Exception_frame)),
      [cpuspoff]  "i" (offsetof(CPU_Exception_frame, register_sp)),
      [v7mlroff]  "i" (offsetof(ARMV7M_Exception_frame, register_lr)),
      [cpuvecoff] "J" (offsetof(CPU_Exception_frame, vector)),
      [cpuvfpoff] "i" (offsetof(CPU_Exception_frame, vfp_context))
  );
}
