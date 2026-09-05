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
# Scope deliberately small. Measured on Linux, the passes run at roughly 100
# files a minute with the oracle off and about 36 with it on; a VM is slower
# again, and the first attempt at lib/msun,lib/libc,bin,usr.bin with
# --ir-limit 400 ran past the 90-minute job limit and reported nothing at all,
# because the report is only written at the end. lib/msun is the 316 files
# that hold every one of the 25 the Linux oracle can see and cannot judge, so
# it is exactly the set worth turning into verdicts first. Widen this once a
# green run has shown what the budget actually buys.
python3 tools/run_todo_passes.py \
    --scope lib/msun \
    --all-passes --skip-corpus \
    --ir-limit 120 --diff-limit 40 \
    --file-timeout 30

echo
echo "== safe-tier comparison"
# The whole question behind SAFE_PASS_NAMES: do the token-level passes
# produce ports that verify at a better rate than the full set? The full run
# above certifies 69 of 120 on lib/msun, and lib/msun averages 0.30 edits per
# file - the ports that verify are largely the ones barely touched. If the
# safe tier verifies at a materially higher rate, the split is the conversion
# path and the span family belongs in proposals for good.
python3 tools/run_todo_passes.py \
    --scope lib/msun \
    --all-passes --safe --skip-corpus \
    --ir-limit 120 --diff-limit 40 \
    --file-timeout 30 > /dev/null
python3 - <<'PY2'
import json
d = json.loads(open("docs/migration/clang_port/pass_report.json").read())
print(f"safe tier: files={d['files']} edits={d['edits_total']} "
      f"IR equal {d['ir_equal']}/{d['ir_ran']}")
PY2

echo
echo "== verified-port ratchet"
ABI_FLOOR_FILE="docs/migration/freebsd_abi_floor.txt"
python3 - "$FLOOR_FILE" "$REPORT" "$ABI_FLOOR_FILE" <<'PY'
import collections, json, pathlib, sys

floor_file, report = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
abi_floor_file = pathlib.Path(sys.argv[3])
d = json.loads(report.read_text())
equal, ran = d["ir_equal"], d["ir_ran"]
abi_equal = d.get("abi_equal", 0)
diff_equal = d.get("diff_equal", 0)

status = collections.Counter(
    r["ir"].get("status") for r in d["records"] if r.get("ir")
)
print(f"files={d['files']} edits={d['edits_total']} refusals={d['refusals_total']}")
print(f"IR equal {equal} / {ran} ran   differential equal {diff_equal}")
for k, v in sorted(status.items(), key=lambda kv: -kv[1]):
    print(f"  {v:5}  {k}")

# A count of mismatches is not a finding until you can see one. Print a few
# with their normalised IR diff so a real semantic break can be told apart
# from yet another C-vs-C++ artefact.
mismatches = [r for r in d["records"] if (r.get("ir") or {}).get("status") == "mismatch"]
for r in mismatches[:4]:
    src = r.get("source") or r.get("path") or "?"
    ir = r["ir"]
    print(f"\n--- mismatch: {src}")
    print(f"    c_lines={ir.get('c_lines')} cxx_lines={ir.get('cxx_lines')}")
    for line in (ir.get("diff") or "(no diff captured)").splitlines()[:24]:
        print(f"    {line}")
if len(mismatches) > 4:
    print(f"\n({len(mismatches) - 4} further mismatches not shown)")

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

# The second, stricter ratchet: same symbols, not just same behaviour.
abi_floor = 0
if abi_floor_file.exists():
    for line in abi_floor_file.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            abi_floor = int(line)
            break

# Name the ports that compute the right answer under the wrong symbol. These
# are the ones that look committable and are not.
abi_broken = [r for r in d["records"]
              if (r.get("ir") or {}).get("equal")
              and not (r.get("ir") or {}).get("abi_equal")]
if abi_broken:
    print(f"\n{len(abi_broken)} port(s) are IR-equal but export different symbols:")
    for r in abi_broken[:6]:
        ir = r["ir"]
        print(f"  {r.get('source') or r.get('path')}")
        print(f"    C only  : {', '.join(ir.get('abi_only_in_c') or []) or '-'}")
        print(f"    C++ only: {', '.join(ir.get('abi_only_in_cxx') or []) or '-'}")
    if len(abi_broken) > 6:
        print(f"  ... and {len(abi_broken) - 6} more")

if abi_equal < abi_floor:
    print(f"\nFAIL {abi_equal} ports are ABI-equal, the floor is {abi_floor}.")
    sys.exit(1)
print(f"\nOK  {abi_equal} ABI-equal, floor {abi_floor}.")
if abi_equal > abi_floor:
    print(f"    Raise {abi_floor_file} to {abi_equal} to lock this in.")
PY
