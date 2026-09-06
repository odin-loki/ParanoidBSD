#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Every instrument, over every scope, then one report.
#
# On the 4-core CI runner this is hours. On 64 cores it is minutes, which
# is the difference between a thing you run when you remember and a thing
# you run before every push.
#
# Usage:  sh tools/verify/run_all.sh [outdir]
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-/tmp/pbsd-verify}"
J="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
mkdir -p "$OUT"
cd "$ROOT"

echo "== $J jobs, output in $OUT"
echo

# Refuse to run if the tools are not actually working. A check that cannot
# see reports zero findings, and this repository has been bitten by that
# three times; a green sweep from broken tools is the worst outcome here.
echo "== proving the instruments work before trusting any number"
if ! sh tools/setup_local.sh >/dev/null 2>&1; then
    echo "FAIL tools/setup_local.sh reports a broken tool. Run it directly." >&2
    exit 1
fi
echo "   ok"
echo

echo "== classifying userland (which functions can be checked, and how)"
python3 tools/verify/classify.py --scope lib/libc --scope lib/msun \
    --jobs "$J" --out "$OUT/classes.json" | tail -8
echo

echo "== classifying the kernel (its own header universe)"
python3 tools/verify/classify.py --scope sys \
    --jobs "$J" --out "$OUT/classes_sys.json" | tail -8
echo

echo "== model checking: SCALAR and VOID, where an unguarded check is sound"
python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
    --classes "$OUT/classes.json" --jobs "$J" \
    --out "$OUT/ub.jsonl" 2>&1 | tail -8
echo

echo "== model checking: POINTER, under a stated precondition"
python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
    --classes "$OUT/classes.json" --allow POINTER --null-depth 3 \
    --jobs "$J" --out "$OUT/ptr.jsonl" 2>&1 | tail -8
echo

echo "== model checking: the kernel"
python3 tools/verify/cbmc_driver.py --scope sys \
    --classes "$OUT/classes_sys.json" --jobs "$J" \
    --out "$OUT/ksys.jsonl" 2>&1 | tail -8
echo

echo "== static analysis: the second instrument"
python3 tools/verify/analyze.py --scope lib/libc --scope lib/msun --scope sys \
    --jobs "$J" --out "$OUT/analyze.jsonl" 2>&1 | tail -6
echo

echo "================================================================"
python3 tools/verify/report.py "$OUT/ub.jsonl" "$OUT/ptr.jsonl" \
    "$OUT/ksys.jsonl" --analyze "$OUT/analyze.jsonl" | tee "$OUT/REPORT.txt"
echo
echo "full report: $OUT/REPORT.txt"
