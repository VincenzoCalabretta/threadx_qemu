#!/usr/bin/env bash
# (c) The Exploration Company
#
# Wrapper for bazel run --run_under with Lauterbach T32.
# This script bridges Bazel's build sandbox with the Nix-provided t32-run tool.
#
# Usage (via Bazel):
#   bazel run --run_under=//tools/t32:t32_run_under //target:binary
#   bazel run --config=m7_debug //app:hello_s32
#
# Bazel passes the built binary path as the last argument.
# We extract it and pass it to t32-run which sets T32_ELF_PATH for load.cmm.

set -euo pipefail

# Last argument is the ELF path from Bazel's sandbox
ELF_PATH="${@: -1}"

# Verify the ELF exists
if [[ ! -f "$ELF_PATH" ]]; then
    echo "Error: ELF not found at $ELF_PATH" >&2
    exit 1
fi

# Resolve to absolute path
ELF_ABS="$(realpath "$ELF_PATH")"

# T32 script directory for M7_0 target
T32_DIR="tools/t32/rdb3_m7_0"

if [[ ! -d "$T32_DIR" ]]; then
    echo "Error: T32 directory not found: $T32_DIR" >&2
    echo "Make sure you're running from the workspace root." >&2
    exit 1
fi

echo "==========================================================="
echo "Lauterbach T32 Debug Session"
echo "==========================================================="
echo "ELF Binary: $ELF_ABS"
echo "Target:     S32G399A M7_0"
echo "Platform:   RDB3"
echo "==========================================================="
echo ""

# Check if t32-run is available (from Nix shell)
if ! command -v t32-run &> /dev/null; then
    echo "Error: t32-run not found in PATH" >&2
    echo "Make sure you're in the Nix development shell:" >&2
    echo "  nix develop" >&2
    exit 1
fi

# Launch T32 via the Nix wrapper, passing ELF path and target directory
exec t32-run "$ELF_ABS" "$T32_DIR"
