#!/usr/bin/env bash
# Burst 13 — private sync + flock green gate (avoids agent races on /tmp/pbsd-repo).
set -euo pipefail
SRC="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST=/tmp/pbsd-b13-src
BUILD=/tmp/pbsd-b13
LOCK=/tmp/pbsd-b13.lock
export PATH="/usr/lib/llvm-18/bin:$PATH"

exec 9>"$LOCK"
flock -w 180 9 || { echo "B13_BUSY"; exit 2; }

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
# 24 = vanished source files during transfer (parallel agents)
[[ "$rc" -eq 0 || "$rc" -eq 24 ]] || exit "$rc"

test -f "$DEST/CMakeLists.txt"
test -d "$DEST/userland"
test -d "$DEST/kernel"
test -d "$DEST/uda"

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
