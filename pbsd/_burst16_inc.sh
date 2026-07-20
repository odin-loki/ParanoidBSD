#!/usr/bin/env bash
# Burst 16 incremental green gate (rsync + autowire + ninja harnesses).
set -euo pipefail
SRC="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST=/tmp/pbsd-b16-src
BUILD=/tmp/pbsd-b16
LOCK=/tmp/pbsd-b16.lock
export PATH="/usr/lib/llvm-18/bin:$PATH"

exec 9>"$LOCK"
flock -w 300 9 || { echo "B16_BUSY"; exit 2; }

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

python3 "$DEST/_fix_cmake_paren.py" || true
python3 "$DEST/_b16_autowire.py" "$DEST" || true
python3 "$DEST/_fix_cmake_paren.py" || true

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
