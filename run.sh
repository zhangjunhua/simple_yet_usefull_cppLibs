#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --config Release

"$BUILD_DIR/bio_example"
"$BUILD_DIR/obj2str_example"
"$BUILD_DIR/util_example"

echo "OK"
