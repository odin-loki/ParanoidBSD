#!/usr/bin/env bash
set -euo pipefail
export PATH="/usr/lib/llvm-18/bin:$PATH"
cd /tmp/pbsd-b16
./pbsd_si_harness
./pbsd_userland_harness
ctest --output-on-failure -R 'si_harness|userland_harness'
HAND=$(find /tmp/pbsd-b16-src -name '*.cppm' ! -path '*/ports/*' 2>/dev/null | wc -l)
echo "HAND_MODULES=$HAND"
echo "WAVE_BUILD_OK"
