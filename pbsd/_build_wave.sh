#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"
rm -rf "$BUILD"
if ! command -v ninja >/dev/null 2>&1; then
  sudo apt-get update -qq && sudo apt-get install -y -qq ninja-build
fi
cmake -S "$ROOT/pbsd" -B "$BUILD" -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD" -j"$(nproc)"
ctest --test-dir "$BUILD" --output-on-failure
python3 "$ROOT/tools/inventory_c_sources.py"
python3 "$ROOT/tools/wave_purge_c_check.py"
echo WAVE_BUILD_OK
