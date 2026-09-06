#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Every instrument, over every scope, then one report.
#
# On the 4-core CI runner this is hours. On a many-core box it is minutes,
# which is the difference between a thing you run when you remember and a
# thing you run before every push.
#
# Usage:  sh tools/verify/run_all.sh [outdir]
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-/tmp/pbsd-verify}"
J="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
mkdir -p "$OUT"
cd "$ROOT"

# Every stage is a long command whose interesting output is its last few
# lines. The obvious spelling of that is
#
#     python3 ... | tail -8
#
# and it is WRONG under set -e: a pipeline's status is the LAST command's,
# so a Python traceback is reported as tail's success. The first run of
# this script on a fresh clone printed five consecutive FileNotFoundErrors
# and carried on to write a report from nothing.
#
# `set -o pipefail` is the usual fix and dash does not have it - which is
# what /bin/sh is on Ubuntu, so the guarded version of that was no fix at
# all. Redirect to a file, CHECK THE STATUS, then tail the file.
stage() {   # stage <label> <log> <command...>
    _label="$1"; _log="$OUT/$2"; shift 2
    echo "== $_label"
    if "$@" > "$_log" 2>&1; then
        tail -8 "$_log" | sed 's/^/  /'
    else
        echo "FAIL: $_label" >&2
        echo "  last 25 lines of $_log:" >&2
        tail -25 "$_log" | sed 's/^/  /' >&2
        exit 1
    fi
    echo
}

echo "== $J jobs, output in $OUT"
echo

# A check that cannot see reports zero findings, and this repository has
# been bitten by that three times. A green sweep from broken instruments is
# the worst outcome available, so this refuses to start.
echo "== proving the instruments work before trusting any number"
if ! sh tools/setup_local.sh > "$OUT/setup.log" 2>&1; then
    echo "FAIL tools/setup_local.sh reports a broken tool:" >&2
    tail -25 "$OUT/setup.log" >&2
    exit 1
fi
echo "   ok"
echo

# docs/port_plan.json is the ledger every stage reads - which file defines
# which functions - and it is GITIGNORED: 6.9MB, generated from the tree by
# tools/port_plan.py, with only the markdown committed. A fresh clone does
# not have it. Takes about a minute.
if [ ! -f docs/port_plan.json ]; then
    echo "== docs/port_plan.json is generated and gitignored; building it"
    echo "   (about a minute, once)"
    stage "the function ledger" "port_plan.log" python3 tools/port_plan.py
fi

stage "classifying userland (which functions can be checked, and how)" \
    "classify.log" \
    python3 tools/verify/classify.py --scope lib/libc --scope lib/msun \
        --jobs "$J" --out "$OUT/classes.json"

stage "classifying the kernel (its own header universe)" \
    "classify_sys.log" \
    python3 tools/verify/classify.py --scope sys \
        --jobs "$J" --out "$OUT/classes_sys.json"

stage "model checking: SCALAR and VOID, where an unguarded check is sound" \
    "ub.log" \
    python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
        --classes "$OUT/classes.json" --jobs "$J" --out "$OUT/ub.jsonl"

stage "model checking: POINTER, under a stated precondition" \
    "ptr.log" \
    python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
        --classes "$OUT/classes.json" --allow POINTER --null-depth 3 \
        --jobs "$J" --out "$OUT/ptr.jsonl"

stage "model checking: the kernel" \
    "ksys.log" \
    python3 tools/verify/cbmc_driver.py --scope sys \
        --classes "$OUT/classes_sys.json" --jobs "$J" --out "$OUT/ksys.jsonl"

stage "static analysis: the second instrument" \
    "analyze.log" \
    python3 tools/verify/analyze.py --scope lib/libc --scope lib/msun \
        --scope sys --jobs "$J" --out "$OUT/analyze.jsonl"

echo "================================================================"
python3 tools/verify/report.py "$OUT/ub.jsonl" "$OUT/ptr.jsonl" \
    "$OUT/ksys.jsonl" --analyze "$OUT/analyze.jsonl" | tee "$OUT/REPORT.txt"
echo
echo "full report: $OUT/REPORT.txt"
echo "raw results: $OUT/*.jsonl"
