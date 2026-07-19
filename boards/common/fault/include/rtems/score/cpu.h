/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Trimmed from RTEMS cpukit/score/cpu/arm/include/rtems/score/cpu.h. Kept
 * only what the ARMv7-M fault dump needs:
 *   - ARM_VFP_context reduced to an opaque type (this project does not
 *     save VFP state; the frame builder stores NULL for vfp_context).
 *   - CPU_Exception_frame for the ARMv7-M layout (no ARCH_V4 union).
 *   - Prototypes for _CPU_Exception_frame_print and _ARM_Exception_default.
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

#ifndef _RTEMS_SCORE_CPU_H
#define _RTEMS_SCORE_CPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t _opaque;
} ARM_VFP_context;

typedef struct {
  union {
    struct {
      uint32_t register_r0;
      uint32_t register_r1;
      uint32_t register_r2;
      uint32_t register_r3;
      uint32_t register_r4;
      uint32_t register_r5;
      uint32_t register_r6;
      uint32_t register_r7;
      uint32_t register_r8;
      uint32_t register_r9;
      uint32_t register_r10;
      uint32_t register_r11;
      uint32_t register_r12;
      uint32_t register_sp;
      void *register_lr;
      void *register_pc;
    };
    uint32_t registers[16];
  };
  uint32_t register_xpsr;
  uint32_t vector;
  const ARM_VFP_context *vfp_context;
  uint32_t reserved_for_stack_alignment;
} CPU_Exception_frame;

void _CPU_Exception_frame_print(const CPU_Exception_frame *frame);

__attribute__((__noreturn__))
void _ARM_Exception_default(CPU_Exception_frame *frame);

#ifdef __cplusplus
}
#endif

#endif /* _RTEMS_SCORE_CPU_H */
