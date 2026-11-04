#!/usr/bin/env bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# Sequential proof campaign: one heavy checker at a time.
set -u

ROOT=/home/odin/ParanoidBSD
OUT=/home/odin/pbsd-sweep
LOG="$OUT/campaign.log"
mkdir -p "$OUT"

exec > >(tee -a "$LOG") 2>&1

echo "== $(date -Is) proof campaign start"
echo "    log=$LOG"
echo "    one heavy checker at a time, in order"

run_step() {
  local label="$1"
  shift
  local script="$1"
  shift
  if [[ -e "$script" ]]; then
    echo
    echo "== $(date -Is) START $label"
    echo "    $script $*"
    if [[ -x "$script" ]]; then
      "$script" "$@"
    else
      bash "$script" "$@"
    fi
    local rc=$?
    echo "== $(date -Is) END $label exit=$rc"
    return "$rc"
  fi
  echo
  echo "== $(date -Is) SKIP $label: $script is not present"
  return 0
}

overall=0
run_step cbmc-retry /home/odin/run-cbmc-retry.sh || overall=$?
run_step esbmc-retry /home/odin/run-esbmc-retry.sh || overall=$?
run_step analyze-retry /home/odin/run-analyze-retry.sh || overall=$?
run_step pointer-null-depth /home/odin/run-pointer-cbmc.sh a || overall=$?
run_step pointer-harness /home/odin/run-pointer-cbmc.sh b || overall=$?

echo
echo "== $(date -Is) report"
cd "$ROOT"
if [[ -f "$OUT/cbmc.jsonl" ]]; then
  python3 -u tools/verify/report.py "$OUT/cbmc.jsonl" || true
else
  echo "FAIL  missing $OUT/cbmc.jsonl"
  overall=1
fi

echo
python3 - <<'PY'
import collections, json
from pathlib import Path

def one(p):
    c = collections.Counter()
    path = Path(p)
    if not path.is_file():
        print(f"{path.name}  missing")
        return
    n = 0
    for line in path.open():
        line = line.strip()
        if not line:
            continue
        try:
            rec = json.loads(line)
        except ValueError:
            c["?"] += 1
            continue
        if rec.get("_meta"):
            continue
        n += 1
        c[rec.get("status", "?")] += 1
    bits = "  ".join(f"{k}={v}" for k, v in sorted(c.items()))
    print(f"{path.name}  {bits}  total={n}")

one("/home/odin/pbsd-sweep/esbmc.jsonl")
one("/home/odin/pbsd-sweep/cbmc-pointer.jsonl")
one("/home/odin/pbsd-sweep/cbmc-harness.jsonl")
one("/home/odin/pbsd-sweep/analyze.jsonl")
PY

echo
echo "== $(date -Is) proof campaign end  overall_exit=$overall"
exit "$overall"
