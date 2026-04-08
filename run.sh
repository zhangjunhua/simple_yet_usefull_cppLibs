#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --config Release

"$BUILD_DIR/bio_example" 2>&1 | tee "$BUILD_DIR/bio_example($(date +%Y-%m-%d_%H-%M-%S)).log"
"$BUILD_DIR/bio_test_basic"

echo "OK"


