#!/bin/sh
# Resume CBMC on functions missing from the live jsonl.
#
# Do NOT pass --retry-status. /home/odin/run-cbmc-retry.sh already dropped
# TIMEOUT/BOUNDED/ERROR from cbmc.jsonl once (2093 rows). A second
# --retry-status pass would drop the BOUNDED/ERROR written since then.
# --resume alone fills the hole. Keep cbmc-old.jsonl as the unwind-16
# evidence. Higher unwind/timeout apply only to the missing pairs.
set -eu

ROOT="${ROOT:-/home/odin/ParanoidBSD}"
OUT="${OUT:-/home/odin/pbsd-sweep}"
JOBS="${JOBS:-8}"
TIMEOUT="${TIMEOUT:-180}"
UNWIND="${UNWIND:-32}"

cd "$ROOT"
export PYTHONUNBUFFERED=1
export PATH="${HOME}/.local/bin:/usr/bin:/bin:${PATH}"

if [ ! -f "$OUT/classes.json" ]; then
	echo "FAIL  missing $OUT/classes.json" >&2
	exit 1
fi
if [ ! -f "$OUT/cbmc.jsonl" ]; then
	echo "FAIL  missing $OUT/cbmc.jsonl" >&2
	exit 1
fi

exec python3 -u "$ROOT/tools/verify/cbmc_driver.py" \
	--scope sys --scope lib --scope bin --scope sbin \
	--scope usr.bin --scope usr.sbin --scope stand --scope contrib \
	--scope crypto --scope cddl --scope secure --scope libexec \
	--classes "$OUT/classes.json" \
	--out "$OUT/cbmc.jsonl" \
	--jobs "$JOBS" \
	--timeout "$TIMEOUT" \
	--unwind "$UNWIND" \
	--resume \
	--allow SCALAR,VOID \
	"$@"
