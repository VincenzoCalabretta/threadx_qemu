/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Trimmed from RTEMS cpukit/score/cpu/arm/include/rtems/score/armv7m.h.
 * Kept only what the fault dump needs: the ARMV7M_Exception_frame layout
 * (8-word HW-stacked frame, no VFP), a truncated ARMV7M_SCB register bank
 * covering ICSR..BFAR with CFSR/HFSR bit macros, and the _ARMV7M_SCB
 * pointer macro. Everything past BFAR is unused by our fault code and has
 * been dropped; the struct layout below still matches the SCB register map
 * from CPUID (0xE000ED00) through BFAR (0xE000ED38).
 *
 * Copyright (c) 2011, 2018 Sebastian Huber <sebastian.huber@embedded-brains.de>
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

#ifndef _RTEMS_SCORE_ARMV7M_H
#define _RTEMS_SCORE_ARMV7M_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The 8-word frame the CPU stacks on exception entry (no VFP extended
 * frame — this project never enables FPU context save). Layout per
 * ARMv7-M ARM (DDI 0403) B1.5.6. */
typedef struct {
  uint32_t register_r0;
  uint32_t register_r1;
  uint32_t register_r2;
  uint32_t register_r3;
  uint32_t register_r12;
  void    *register_lr;
  void    *register_pc;
  uint32_t register_xpsr;
} ARMV7M_Exception_frame;

typedef struct {
  uint32_t cpuid;               /* 0xE000ED00 */
  uint32_t icsr;                /* 0xE000ED04 */
  uint32_t vtor;                /* 0xE000ED08 */
  uint32_t aircr;               /* 0xE000ED0C */
  uint32_t scr;                 /* 0xE000ED10 */
  uint32_t ccr;                 /* 0xE000ED14 */
  uint8_t  shpr[12];            /* 0xE000ED18 */

#define ARMV7M_SCB_SHCSR_USGFAULTENA (1U << 18)
#define ARMV7M_SCB_SHCSR_BUSFAULTENA (1U << 17)
#define ARMV7M_SCB_SHCSR_MEMFAULTENA (1U << 16)
  uint32_t shcsr;               /* 0xE000ED24 */

#define ARMV7M_SCB_CFSR_MMFSR_MASK 0xff
#define ARMV7M_SCB_CFSR_MMFSR_GET(n) ((n) & ARMV7M_SCB_CFSR_MMFSR_MASK)
#define ARMV7M_SCB_CFSR_BFSR_MASK 0xff00
#define ARMV7M_SCB_CFSR_BFSR_GET(n) ((n) & ARMV7M_SCB_CFSR_BFSR_MASK)
#define ARMV7M_SCB_CFSR_UFSR_MASK 0xffff0000
#define ARMV7M_SCB_CFSR_UFSR_GET(n) ((n) & ARMV7M_SCB_CFSR_UFSR_MASK)
  uint32_t cfsr;                /* 0xE000ED28 */

#define ARMV7M_SCB_HFSR_VECTTBL_MASK 0x2
#define ARMV7M_SCB_HFSR_FORCED_MASK (1U << 30)
#define ARMV7M_SCB_HFSR_DEBUGEVT_MASK (1U << 31)
  uint32_t hfsr;                /* 0xE000ED2C */

  uint32_t dfsr;                /* 0xE000ED30 */
  uint32_t mmfar;               /* 0xE000ED34 */
  uint32_t bfar;                /* 0xE000ED38 */
} ARMV7M_SCB;

#define ARMV7M_SCS_BASE  0xE000E000
#define ARMV7M_SCB_BASE  (ARMV7M_SCS_BASE + 0xD00)
#define _ARMV7M_SCB      ((volatile ARMV7M_SCB *) ARMV7M_SCB_BASE)

#ifdef __cplusplus
}
#endif

#endif /* _RTEMS_SCORE_ARMV7M_H */
