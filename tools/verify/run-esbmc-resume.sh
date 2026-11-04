#!/bin/sh
# Resume ESBMC on functions missing from the live jsonl.
#
# Do NOT pass --retry-status. The ERROR retry already dropped ERROR rows
# once; a second pass would drop the kinduction results written since.
# --resume alone fills the hole. Keep esbmc.jsonl as the live file.
set -eu

ROOT="${ROOT:-/home/odin/ParanoidBSD}"
OUT="${OUT:-/home/odin/pbsd-sweep}"
JOBS="${JOBS:-8}"
TIMEOUT="${TIMEOUT:-60}"

cd "$ROOT"
export PYTHONUNBUFFERED=1
export PATH="${HOME}/.local/bin:/usr/bin:/bin:${PATH}"

if [ ! -f "$OUT/classes.json" ]; then
	echo "FAIL  missing $OUT/classes.json" >&2
	exit 1
fi
if [ ! -f "$OUT/esbmc.jsonl" ]; then
	echo "FAIL  missing $OUT/esbmc.jsonl" >&2
	exit 1
fi

exec python3 -u "$ROOT/tools/verify/esbmc_driver.py" \
	--scope sys --scope lib \
	--classes "$OUT/classes.json" \
	--out "$OUT/esbmc.jsonl" \
	--pp-dir "$OUT/esbmc-i" \
	--mode kinduction \
	--jobs "$JOBS" \
	--timeout "$TIMEOUT" \
	--resume \
	--allow SCALAR,VOID \
	"$@"
