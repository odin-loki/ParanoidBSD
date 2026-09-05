#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Run the mechanical phase's IR-equivalence oracle on a FreeBSD host.
#
# On Linux the oracle cannot certify a single real port: math_private.h wants
# FreeBSD's type environment, so uint32_t comes up undeclared and
# ieee_quad_shape_type redefines against glibc's. That is the same wall that
# keeps 148 modules in docs/migration/linux_build_exceptions.txt, and no
# arrangement of -I flags gets past it. Here the headers are native, so a
# staged .cpp can finally be compared against the C it came from.
#
# Two gates:
#   1. the golden corpus must pass, IR included - if it does not, the toolchain
#      is wrong and nothing below means anything;
#   2. the number of real ports proved IR-equivalent must not fall below the
#      floor in docs/migration/freebsd_verified_floor.txt.
#
# The floor starts at 0, so this cannot fail on its first run. Raise it as the
# count climbs; that is the ratchet.
set -eu

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT"

FLOOR_FILE=docs/migration/freebsd_verified_floor.txt
REPORT=docs/migration/clang_port/pass_report.json

echo "== toolchain"
clang --version | head -1
python3 --version
uname -a

echo
echo "== golden corpus (hard gate)"
python3 tools/run_todo_passes.py --corpus-only > /tmp/corpus.json
python3 - <<'PY'
import json, sys
d = json.load(open("/tmp/corpus.json"))
print(f"corpus ok={d['ok']} golden_ok={d.get('golden_ok')} "
      f"ir_ok={d.get('ir_ok')} ir={d.get('ir_equal')}/{d.get('ir_ran')}")
if not d["ok"]:
    bad = [c["case"] for c in d["cases"] if not c["ok"]]
    print("FAIL corpus did not pass on FreeBSD:", bad or "IR mismatch")
    sys.exit(1)
PY

echo
echo "== IR oracle on real sources"
python3 tools/run_todo_passes.py \
    --scope lib/msun,lib/libc,bin,usr.bin \
    --all-passes --skip-corpus \
    --ir-limit 400 --diff-limit 200

echo
echo "== verified-port ratchet"
python3 - "$FLOOR_FILE" "$REPORT" <<'PY'
import collections, json, pathlib, sys

floor_file, report = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
d = json.loads(report.read_text())
equal, ran = d["ir_equal"], d["ir_ran"]
diff_equal = d.get("diff_equal", 0)

status = collections.Counter(
    r["ir"].get("status") for r in d["records"] if r.get("ir")
)
print(f"files={d['files']} edits={d['edits_total']} refusals={d['refusals_total']}")
print(f"IR equal {equal} / {ran} ran   differential equal {diff_equal}")
for k, v in sorted(status.items(), key=lambda kv: -kv[1]):
    print(f"  {v:5}  {k}")

floor = 0
if floor_file.exists():
    for line in floor_file.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            floor = int(line)
            break

if equal < floor:
    print(f"\nFAIL {equal} ports verify, the floor is {floor}. A port that was "
          f"proved IR-equivalent no longer is.")
    sys.exit(1)
print(f"\nOK  {equal} verified, floor {floor}.")
if equal > floor:
    print(f"    Raise {floor_file} to {equal} to lock this in.")
PY
