/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup ErrorPanicSupport
 *
 * @brief This source file contains the implementation of rtems_panic().
 */

/*
 * Local rewrite of RTEMS cpukit/sapi/src/panic.c. Upstream tail-calls
 * _Terminate (the RTEMS fatal-extension chain), which we do not link
 * against — so we print the formatted message, then hand off to the
 * same halt loop the fault handler uses (WFI in an infinite loop; see
 * arm_exception_default.c for the WFI-over-BKPT rationale).
 *
 * Copyright (c) 2017 embedded brains GmbH & Co. KG
 */

#include <stdarg.h>

#include <rtems/bspIo.h>
#include <rtems/fatal.h>

__attribute__((__noreturn__))
void rtems_panic(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintk(fmt, ap);
  va_end(ap);
  rtems_putc('\n');

  for (;;) {
    __asm__ volatile ("wfi");
  }
}
