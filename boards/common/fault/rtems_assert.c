/* SPDX-License-Identifier: BSD-2-Clause */

/**
 *  @file
 *
 *  @brief Evaluate Assertion — overrides Newlib's default __assert_func so
 *    that `#include <assert.h>` failures print via printk and halt via the
 *    same path as rtems_panic().
 *  @ingroup libcsupport
 */

/*
 * Trimmed from RTEMS cpukit/libcsupport/src/__assert.c:
 *   - The RTEMS_NEWLIB / HAVE___ASSERT_FUNC guards are dropped; we build
 *     against the arm-none-eabi Newlib profile unconditionally.
 *   - The upstream call rtems_fatal(RTEMS_FATAL_SOURCE_ASSERT, ...) is
 *     replaced with a rtems_panic() tail — RTEMS's fatal-source chain
 *     is not linked in this build. The printed message is unchanged.
 *
 *  COPYRIGHT (c) 2007-2013.
 *  On-Line Applications Research Corporation (OAR).
 */

#include <assert.h>

#include <rtems/bspIo.h>
#include <rtems/fatal.h>

void __assert_func(
  const char *file,
  int         line,
  const char *func,
  const char *failedexpr
)
{
  rtems_panic(
    "assertion \"%s\" failed: file \"%s\", line %d%s%s",
    failedexpr,
    file,
    line,
    (func) ? ", function: " : "",
    (func) ? func : ""
  );
}
