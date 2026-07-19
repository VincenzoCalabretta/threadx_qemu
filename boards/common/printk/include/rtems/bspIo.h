/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Trimmed from RTEMS cpukit/include/rtems/bspIo.h — kept only the pieces
 * our vendored printk chain needs: the two BSP function-pointer typedefs,
 * their externs, and the printk/rtems_putc/vprintk prototypes.
 *
 * Copyright (C) 1998 Ralf Corsepius (corsepiu@faw.uni-ulm.de)
 * Copyright (C) 2020 embedded brains GmbH & Co. KG
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

#ifndef _RTEMS_BSPIO_H
#define _RTEMS_BSPIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*BSP_output_char_function_type)(char);
typedef int  (*BSP_polling_getchar_function_type)(void);

extern BSP_output_char_function_type      BSP_output_char;
extern BSP_polling_getchar_function_type  BSP_poll_char;

void rtems_putc(char c);

/* Adapter from the IO_Put_char (int, void*) signature used by _IO_Vprintf
 * to rtems_putc(char). Declared here to keep bspIo.h a single header for
 * the whole printk chain. */
void rtems_put_char(int c, void *unused);

int printk(const char *fmt, ...)
    __attribute__((__format__(__printf__, 1, 2)));

int vprintk(const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* _RTEMS_BSPIO_H */
