#!/usr/bin/env bash
set -euo pipefail

# Simple build helper. Use this instead of compiling headers directly.
# Usage: ./build.sh [target]
# Targets: all (default), solver, clean

TARGET=${1:-all}

# Don't allow building a header file directly (helps catch accidental commands)
if [[ "$TARGET" == *.h ]]; then
  echo "ERROR: trying to build a header directly. Use 'make' or build a .cpp file, not a .h file." >&2
  exit 2
fi

make -C "$(dirname "$0")" $TARGET
