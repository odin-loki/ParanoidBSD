#!/bin/sh
# After clang --analyze retry: rebuild goto binaries off /tmp, then
# CBMC-retry SCALAR/VOID TIMEOUT/BOUNDED/ERROR. Campaign calls this
# after ESBMC so the heavy checkers stay sequential.
set -eu

ROOT="${ROOT:-/home/odin/ParanoidBSD}"
PREV="${PREV:-/home/odin/pbsd-sweep/analyze.jsonl}"
OUT="${OUT:-/home/odin/pbsd-sweep/analyze.jsonl}"
JOBS="${JOBS:-32}"
TIMEOUT="${TIMEOUT:-120}"
GOTO="${GOTO:-/home/odin/pbsd-sweep/goto}"
CLASSES="${CLASSES:-/home/odin/pbsd-sweep/classes.json}"

cd "$ROOT"
export PYTHONUNBUFFERED=1
export PATH="${HOME}/.local/bin:/usr/bin:/bin:${PATH}"

if [ ! -f "$PREV" ]; then
	echo "missing previous results: $PREV" >&2
	exit 1
fi

python3 -c "
import sys
sys.path.insert(0, 'tools/verify')
import includes, userland_names
includes.incs_shim('amd64')
userland_names.warm(('amd64',))
print('warmed incs_shim + userland names')
"

python3 tools/verify/analyze.py \
	--scope sys --scope lib --scope bin --scope sbin \
	--scope usr.bin --scope usr.sbin --scope stand \
	--scope contrib --scope crypto --scope cddl \
	--scope secure --scope libexec \
	--jobs "$JOBS" --timeout "$TIMEOUT" \
	--out "$OUT" --resume

mkdir -p "$GOTO"
echo "== classify --refresh-gb -> $GOTO"
python3 tools/verify/classify.py \
	--scope sys --scope lib --scope bin --scope sbin \
	--scope usr.bin --scope usr.sbin --scope stand \
	--scope contrib --scope crypto --scope cddl \
	--scope secure --scope libexec \
	--jobs "$JOBS" --timeout 120 \
	--outdir "$GOTO" --out "$CLASSES" \
	--resume --refresh-gb

echo "== cbmc-retry now that goto binaries exist"
exec /home/odin/run-cbmc-retry.sh
