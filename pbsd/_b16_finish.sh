#!/usr/bin/env bash
set -euo pipefail
export PATH="/usr/lib/llvm-18/bin:$PATH"
SRC="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST=/tmp/pbsd-b16-src
BUILD=/tmp/pbsd-b16
LOCK=/tmp/pbsd-b16.lock
LOG=/tmp/pbsd-b16-build.log

exec 9>"$LOCK"
flock -w 180 9 || { echo "B16_BUSY"; exit 2; }

mkdir -p "$DEST" "$BUILD"
set +e
rsync -a \
  --exclude 'ports/' \
  --exclude '_build*/' --exclude 'build-*/' --exclude 'build/' \
  --exclude '.git/' \
  --exclude '*.o' \
  --exclude 'CMakeFiles/' \
  "$SRC/" "$DEST/" >"$LOG" 2>&1
rc=$?
set -e
[[ "$rc" -eq 0 || "$rc" -eq 24 ]] || { tail -50 "$LOG"; exit "$rc"; }

python3 "$DEST/_fix_cmake_paren.py" >>"$LOG" 2>&1 || true
python3 "$DEST/_b16_autowire.py" "$DEST" >>"$LOG" 2>&1 || true
python3 "$DEST/_fix_cmake_paren.py" >>"$LOG" 2>&1 || true

cd "$BUILD"
cmake -G Ninja \
  -DCMAKE_C_COMPILER=/usr/bin/clang-18 \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 \
  -DCMAKE_BUILD_TYPE=Debug \
  "$DEST" >>"$LOG" 2>&1
echo "CMAKE_OK"

# Quiet progress: ninja to log only
set +e
ninja -j"$(nproc)" pbsd_si_harness pbsd_userland_harness >>"$LOG" 2>&1
nrc=$?
set -e
if [[ "$nrc" -ne 0 ]]; then
  echo "NINJA_FAIL=$nrc"
  grep -E 'error:|FAILED:|fatal error' "$LOG" | tail -40
  exit "$nrc"
fi
echo "NINJA_OK"

ctest --output-on-failure -R 'si_harness|userland_harness' | tee -a "$LOG"
HAND=$(find "$DEST" -name '*.cppm' ! -path '*/ports/*' | wc -l)
echo "HAND_MODULES=$HAND"
echo "WAVE_BUILD_OK"
