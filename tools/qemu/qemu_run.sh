#!/usr/bin/env bash
# `bazel run --config=qemu //app:hello_qemu` invokes this script with the ELF
# path as the first argument. The nix devShell exports NIX_QEMU_BIN, which
# points at qemu-system-arm inside the nix store — no host PATH lookup.
set -euo pipefail

if [ -z "${NIX_QEMU_BIN:-}" ]; then
    echo "qemu_run: NIX_QEMU_BIN is unset. Enter the nix devShell first: 'nix develop'." >&2
    exit 1
fi

if [ $# -lt 1 ]; then
    echo "usage: qemu_run <elf>" >&2
    exit 1
fi

elf="$1"
shift

exec "$NIX_QEMU_BIN" \
    -M mps2-an500 \
    -nographic \
    -kernel "$elf" \
    "$@"
