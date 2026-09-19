#!/bin/sh
# POINTER CBMC over sys+lib. --min-null-tree-depth is not a theorem.
#
# Pass a: every POINTER function, CBMC --min-null-tree-depth 3.
#         A clean result is PROVED-ASSUMING, never PROVED.
#         Measured: ~101632 POINTER functions (ledger ∩ classes, sys+lib).
#         This is hours. Do not start it by accident.
#
# Pass b: string-like POINTER only (char * / wchar_t * without a size).
#         Generated C harness, CBMC --function harness, --limit 200.
#         First cut is lib/libc/string then sys/kern. Also PROVED-ASSUMING.
#
#   /home/odin/run-pointer-cbmc.sh a
#   /home/odin/run-pointer-cbmc.sh b
#   /home/odin/run-pointer-cbmc.sh all
#
# No default. A bare invocation prints this and exits 2.
set -eu

ROOT="${ROOT:-/home/odin/ParanoidBSD}"
CLASSES="${CLASSES:-/home/odin/pbsd-sweep/classes.json}"
OUTDIR="${OUTDIR:-/home/odin/pbsd-sweep}"
JOBS="${JOBS:-24}"
TIMEOUT="${TIMEOUT:-60}"
HARNESS_LIMIT="${HARNESS_LIMIT:-200}"
HARNESS_DIR="${HARNESS_DIR:-$OUTDIR/harness-c}"

cd "$ROOT"

if [ ! -f "$CLASSES" ]; then
	echo "missing classes.json: $CLASSES" >&2
	echo "classify.py writes it; this script does not re-run that." >&2
	exit 1
fi

run_a() {
	# Exact contract: POINTER, null-depth 3, sys+lib, resumable.
	# Never call a success here PROVED — cbmc_driver.py records
	# PROVED-ASSUMING when --null-depth is set.
	python3 tools/verify/cbmc_driver.py \
		--allow POINTER \
		--null-depth 3 \
		--scope sys --scope lib \
		--jobs "$JOBS" \
		--timeout "$TIMEOUT" \
		--out "$OUTDIR/cbmc-pointer.jsonl" \
		--classes "$CLASSES" \
		--resume
}

run_b() {
	mkdir -p "$HARNESS_DIR"
	# Optional string-harness pass. --limit 200 so an unattended
	# first cut cannot run for days. libc string first, then sys/kern.
	python3 tools/verify/harness.py \
		--scope lib/libc/string --scope sys/kern \
		--jobs "$JOBS" \
		--timeout "$TIMEOUT" \
		--limit "$HARNESS_LIMIT" \
		--harness-dir "$HARNESS_DIR" \
		--out "$OUTDIR/cbmc-harness.jsonl" \
		--classes "$CLASSES" \
		--resume
}

case "${1:-}" in
a)	run_a ;;
b)	run_b ;;
all)	run_a; run_b ;;
*)
	echo "usage: $0 a|b|all" >&2
	echo "  a    POINTER --null-depth 3, scope sys+lib (~101k functions)" >&2
	echo "  b    string harnesses, lib/libc/string + sys/kern, --limit ${HARNESS_LIMIT}" >&2
	echo "  all  a then b" >&2
	echo "This script does not default: pass a is multi-hour." >&2
	exit 2
	;;
esac
