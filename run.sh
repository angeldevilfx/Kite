#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_PATH="$ROOT_DIR/$BUILD_DIR"

if [[ -f "$BUILD_PATH/CMakeCache.txt" ]]; then
  cached_source_dir="$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$BUILD_PATH/CMakeCache.txt" | tail -n 1)"
  if [[ -n "$cached_source_dir" && "$cached_source_dir" != "$ROOT_DIR" ]]; then
    rm -rf "$BUILD_PATH"
  fi
fi

cmake -S "$ROOT_DIR" -B "$BUILD_PATH" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_PATH" --parallel

exec "$BUILD_PATH/kitebrowser" "$@"
