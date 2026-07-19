# Generated BUILD symlinked in by //third_party/threadx:threadx.bzl.
# Exposes Eclipse ThreadX (common kernel + cortex-m7/GNU port) as one
# cc_library. tx_misra.S is excluded — MISRA compliance is an opt-in mode
# we do not build here.

load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "threadx",
    srcs = glob(
        [
            "common/src/*.c",
            "ports/cortex_m7/gnu/src/*.S",
        ],
        exclude = ["ports/cortex_m7/gnu/src/tx_misra.S"],
    ),
    hdrs = glob([
        "common/inc/*.h",
        "ports/cortex_m7/gnu/inc/*.h",
    ]),
    includes = [
        "common/inc",
        "ports/cortex_m7/gnu/inc",
    ],
    # ThreadX common sources trip a couple of GCC pedantic warnings that are
    # fine for a vendored kernel; silence rather than patch upstream.
    copts = [
        "-Wno-unused-parameter",
        "-Wno-unused-but-set-variable",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-m",
    ],
)
