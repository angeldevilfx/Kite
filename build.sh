#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_PATH="$ROOT_DIR/$BUILD_DIR"

rm -rf "$BUILD_PATH"
cmake -S "$ROOT_DIR" -B "$BUILD_PATH" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_PATH" --parallel
