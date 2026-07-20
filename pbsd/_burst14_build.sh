#!/usr/bin/env bash
# Burst 14 — private sync + flock green gate.
set -euo pipefail
SRC="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST=/tmp/pbsd-b14-src
BUILD=/tmp/pbsd-b14
LOCK=/tmp/pbsd-b14.lock
export PATH="/usr/lib/llvm-18/bin:$PATH"

exec 9>"$LOCK"
flock -w 180 9 || { echo "B14_BUSY"; exit 2; }

rm -rf "$DEST" "$BUILD"
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

test -f "$DEST/CMakeLists.txt"
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
