#!/usr/bin/env bash
# Burst 13 incremental: rsync + ninja without wiping build cache.
set -euo pipefail
SRC="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST=/tmp/pbsd-b13-src
BUILD=/tmp/pbsd-b13
LOCK=/tmp/pbsd-b13.lock
export PATH="/usr/lib/llvm-18/bin:$PATH"

exec 9>"$LOCK"
flock -w 180 9 || { echo "B13_BUSY"; exit 2; }

mkdir -p "$DEST" "$BUILD"
set +e
rsync -a \
  --exclude 'ports/' \
  --exclude '_build*/' --exclude 'build-*/' --exclude 'build/' \
  --exclude '.git/' \
  --exclude '*.o' \
  --exclude 'CMakeFiles/' \
  "$SRC/" "$DEST/"
rc=$?
set -e
[[ "$rc" -eq 0 || "$rc" -eq 24 ]] || exit "$rc"

cd "$BUILD"
cmake -G Ninja \
  -DCMAKE_C_COMPILER=/usr/bin/clang-18 \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 \
  -DCMAKE_BUILD_TYPE=Debug \
  "$DEST"
ninja -j"$(nproc)" pbsd_si_harness pbsd_userland_harness
ctest --output-on-failure -R 'si_harness|userland_harness'
HAND=$(find "$DEST" -name '*.cppm' ! -path '*/ports/*' | wc -l)
echo "HAND_MODULES=$HAND"
echo "WAVE_BUILD_OK"
