/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Trimmed from RTEMS cpukit/include/rtems/fatal.h — kept only the pieces
 * used by rtems_panic() and __assert_func(): the panic prototype and the
 * rtems_assert_context struct.
 *
 * Copyright (C) 2014, 2020 embedded brains GmbH & Co. KG
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

#ifndef _RTEMS_FATAL_H
#define _RTEMS_FATAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *file;
  int         line;
  const char *function;
  const char *failed_expression;
} rtems_assert_context;

/**
 * @brief Prints the formatted message via printk() and halts.
 *
 * Upstream RTEMS invokes the fatal extension chain here; this local
 * build has no scheduler / extension chain, so we just parking-loop.
 */
__attribute__((__noreturn__, __format__(__printf__, 1, 2)))
void rtems_panic(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _RTEMS_FATAL_H */
