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
# it is exactly the set worth turning into verdicts first.
#
# "Widen this once a green run has shown what the budget actually buys" is
# what this comment used to end with, and green runs have now shown it: the
# whole job, all four phases, takes under four minutes of a 120-minute
# timeout. Meanwhile --ir-limit 120 against 321 files meant two thirds of
# lib/msun got the verdict `skipped_budget` and "88 of 120" was 88 of the
# first 120 eligible files, not of the library. Every port outside that
# prefix was unjudged, so the committable set was drawn from a third of the
# scope for no reason but a cap nobody had revisited.
#
# The budget is spent in discovery order and is deterministic, so raising it
# can only add verdicts, never withdraw one - the floors below stay valid.
# lib/libc stays at 120: it is 1,220 files, it is reported rather than
# ratcheted, and it is not the scope anything is being ported from yet.
python3 tools/run_todo_passes.py \
    --scope lib/msun \
    --all-passes --skip-corpus \
    --ir-limit 400 --diff-limit 40 \
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
    --ir-limit 400 --diff-limit 40 \
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
python3 - "$FLOOR_FILE" "$REPORT" "$ABI_FLOOR_FILE" hbsd/src/lib/msun <<'PY'
import collections, json, pathlib, sys

floor_file, report = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
abi_floor_file = pathlib.Path(sys.argv[3])
scope_dir = pathlib.Path(sys.argv[4])
d = json.loads(report.read_text())

# A port that has been COMMITTED leaves the candidate set.
#
# discover_sources() globs *.c, so the moment lib/msun/src/k_cos.c becomes
# k_cos.cpp it is no longer discovered, no longer compiled, no longer
# compared - and ir_equal falls by one. The ratchet below would read the
# first successful port in this project's history as "a port that was proved
# IR-equivalent no longer is" and fail the job.
#
# That is backwards. A committed port is a stronger statement than a
# verified one: it was verified AND it built. So committed ports are counted
# and the floors are checked against the sum. A genuine regression - a file
# that is still a .c and stopped verifying - still lands below the floor and
# still fails, which is what the ratchet is for.
ported = sorted(p.as_posix() for p in scope_dir.rglob("*.cpp"))
equal, ran = d["ir_equal"], d["ir_ran"]
abi_equal = d.get("abi_equal", 0)
diff_equal = d.get("diff_equal", 0)

status = collections.Counter(
    r["ir"].get("status") for r in d["records"] if r.get("ir")
)
print(f"files={d['files']} edits={d['edits_total']} refusals={d['refusals_total']}")
print(f"IR equal {equal} / {ran} ran   differential equal {diff_equal}")
print(f"committed ports in {scope_dir}: {len(ported)}")
for s_ in ported:
    print(f"  {s_}")
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

def _read_floor(path):
    if path.exists():
        for line in path.read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#"):
                return int(line)
    return 0


# Both floors are read here rather than one at each check, so the summary
# below can name them before either check can exit.
floor = _read_floor(floor_file)
abi_floor = _read_floor(abi_floor_file)

total = equal + len(ported)
abi_total_pre = abi_equal + len(ported)

# Everything above is hundreds of lines by the time the committable list is
# printed, and the numbers that matter end up buried in the middle. Stash
# them for the SUMMARY block at the end, so reading a run is a tail and not
# a search - and do it BEFORE the floor checks, because a run that fails a
# floor is exactly the run whose numbers someone wants.
pathlib.Path("/tmp/pbsd_ratchet.txt").write_text(
    f"  msun ratchet:  {equal} verified + {len(ported)} committed = {total} "
    f"(floor {floor})\n"
    f"  msun ABI:      {abi_equal} ABI-equal + {len(ported)} committed = "
    f"{abi_total_pre} (floor {abi_floor})\n")

if total < floor:
    print(f"\nFAIL {equal} ports verify and {len(ported)} are committed, "
          f"{total} against a floor of {floor}. A port that was proved "
          f"IR-equivalent no longer is, and it was not committed either.")
    sys.exit(1)
print(f"\nOK  {equal} verified + {len(ported)} committed = {total}, "
      f"floor {floor}.")
if total > floor:
    print(f"    Raise {floor_file} to {total} to lock this in.")

# The second, stricter ratchet: same symbols, not just same behaviour.
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

abi_total = abi_equal + len(ported)
if abi_total < abi_floor:
    print(f"\nFAIL {abi_equal} ports are ABI-equal and {len(ported)} are "
          f"committed, {abi_total} against a floor of {abi_floor}.")
    sys.exit(1)
print(f"\nOK  {abi_equal} ABI-equal + {len(ported)} committed = {abi_total}, "
      f"floor {abi_floor}.")
if abi_total > abi_floor:
    print(f"    Raise {abi_floor_file} to {abi_total} to lock this in.")
PY

echo
echo "== wider scope"
# lib/msun is 321 of the tree's 29,579 C files, and it was chosen because it
# was the only scope the Linux oracle could say anything about - FreeBSD's
# math_private.h and glibc's type environment could not both be right at once.
# That constraint went away when this job got a FreeBSD host, and the boundary
# stayed where it was. lib/libc is 1,220 files and is the next thing anything
# links against.
#
# Reported, not ratcheted. The floors are per-scope by construction, and a
# floor set against 321 files says nothing about 1,220.
#
# --ir-limit covers the scope, for the reason it now does for lib/msun: a
# cap smaller than the scope does not mean the rest failed, it means the
# rest was recorded `skipped_budget` and never compiled. On lib/msun the
# same cap was hiding 86 ports that had been verifying all along, and
# "37 of 120" here is 37 of the first tenth of the library in discovery
# order. The whole job took four minutes of a 120-minute timeout before
# lib/msun was widened and six after.
python3 tools/run_todo_passes.py \
    --scope lib/libc \
    --all-passes --safe --skip-corpus \
    --ir-limit 1500 --diff-limit 40 \
    --file-timeout 30 > /dev/null || true
python3 - <<'PY2'
import json, pathlib
p = pathlib.Path("docs/migration/clang_port/pass_report.json")
if not p.exists():
    print("lib/libc: no report produced")
else:
    d = json.loads(p.read_text())
    print(f"lib/libc: files={d['files']} edits={d['edits_total']} "
          f"refusals={d.get('refusals_total', 0)}")
    print(f"          IR equal {d['ir_equal']}/{d['ir_ran']}  "
          f"ABI equal {d.get('abi_equal', 0)}")

    # Name the ports that compute the same thing and export different
    # symbols. That gap is always the same cause - a function that is
    # external but declared in no header, so C++ mangles it and the library
    # would ship it under a new name - and the fix is a __BEGIN_DECLS in
    # whichever header should have declared it. Counting them says there is
    # work; naming them says where.
    split = [r for r in d.get("records", [])
             if (r.get("ir") or {}).get("equal")
             and not (r.get("ir") or {}).get("abi_equal")]
    if split:
        print(f"\n          {len(split)} port(s) IR-equal but not ABI-equal:")
        for r in split:
            ir = r["ir"]
            print(f"            {r.get('source', r.get('case', '?'))}")
            for s in ir.get("abi_only_in_c", [])[:6]:
                print(f"              only in C:   {s}")
            for s in ir.get("abi_only_in_cxx", [])[:6]:
                print(f"              only in C++: {s}")
    elif d.get("ir_equal", 0) > 0:
        print("\n          every IR-equal port is also ABI-equal.")
    else:
        print("\n          no port was IR-equal, so there is nothing to "
              "compare ABIs on.")

    # Name the committable ones, for the same reason lib/msun's are named:
    # a count cannot be acted on. Same strict definition - IR-equal and
    # ABI-equal - though without target flags here, since this phase does
    # not run under them.
    ready = sorted((r.get("source") or "?", r.get("edits", 0))
                   for r in d.get("records", [])
                   if (r.get("ir") or {}).get("equal")
                   and (r.get("ir") or {}).get("abi_equal"))
    zero = [s_ for s_, e_ in ready if e_ == 0]
    print(f"\n          committable: {len(ready)}, of which {len(zero)} "
          f"are zero-edit")
    for s_, e_ in ready:
        mark = "" if e_ == 0 else f"   ({e_} edit(s) - NOT a pure rename)"
        print(f"            {s_}{mark}")
    # The same `+ committed` term lib/msun's ratchet has, and for the same
    # reason: a port that lands stops being a .c, discover_sources() stops
    # finding it, and ir_equal falls by one. Without the sum, landing a port
    # reads as losing one - which is precisely the shape that would make a
    # floor here punish progress. gen/isatty.cpp is the first.
    libc_dir = pathlib.Path("hbsd/src/lib/libc")
    committed = sorted(q.as_posix() for q in libc_dir.rglob("*.cpp"))
    ir_total = d["ir_equal"] + len(committed)
    abi_total = d.get("abi_equal", 0) + len(committed)
    print(f"\n          committed: {len(committed)}")
    for c_ in committed:
        print(f"            {c_}")
    pathlib.Path("/tmp/pbsd_libc.txt").write_text(
        f"  lib/libc IR:   {d['ir_equal']} verified + {len(committed)} "
        f"committed = {ir_total} of {d['ir_ran']} ran\n"
        f"  lib/libc ABI:  {d.get('abi_equal', 0)} ABI-equal + "
        f"{len(committed)} committed = {abi_total}  "
        f"committable {len(ready)}\n")
PY2

echo
echo "== target-flag comparison"
# The two floors above are measured with the oracle's own flags. lib/msun is
# built with -ffp-exception-behavior=maytrap and -fno-math-errno, and a math
# library proved equivalent without those has been proved equivalent to
# something the tree does not build. This runs the same scope with the
# semantic CFLAGS the real build would use, on both sides, and reports the
# number. It does not gate yet - the floors were set against the other
# configuration, and moving both at once would say nothing about either.
PBSD_TARGET_FLAGS=1 python3 tools/run_todo_passes.py \
    --scope lib/msun \
    --all-passes --safe --skip-corpus \
    --ir-limit 400 --diff-limit 40 \
    --file-timeout 30 > /dev/null
python3 - <<'PY2'
import json
d = json.loads(open("docs/migration/clang_port/pass_report.json").read())
print(f"under target flags: files={d['files']} edits={d['edits_total']} "
      f"IR equal {d['ir_equal']}/{d['ir_ran']}  ABI equal {d.get('abi_equal', 0)}")
seen = set()
for r in d["records"]:
    tf = tuple((r.get("ir") or {}).get("target_flags") or ())
    if tf and tf not in seen:
        seen.add(tf)
        print("  flags used:", " ".join(tf))
if not seen:
    print("  no target flags resolved - check tools/pbsd_passes/target_flags.py")

# Name them.
#
# Every number this job prints is a count, and a count cannot be acted on.
# docs/migration/COMMITTING_PORTS.md says what it takes to land a port and
# then cannot say WHICH port, because 88 of 120 does not name one; the first
# real port was going to be picked by guessing which file the 88 contained.
#
# This is the committable set and the definition is deliberately strict:
# IR-equal AND ABI-equal AND measured under the flags lib/msun is actually
# built with. Anything short of all three is a port that verifies against a
# build configuration nothing ships, which is the mistake the floor comment
# already records once.
# The edit count comes with it, because "committable" and "a rename is
# enough" are different claims and only the second one is a one-line commit.
# The oracle verifies the STAGED .cpp; where the pass made edits, the staged
# file is not the .c and renaming the .c does not reproduce what was
# measured. The hundred-file lib/msun batch selected on `edits == 0` for
# exactly this reason and the list it selected from did not say so, which
# meant re-deriving it by hand from the report.
ready = sorted((r.get("source") or "?", r.get("edits", 0))
               for r in d["records"]
               if (r.get("ir") or {}).get("equal")
               and (r.get("ir") or {}).get("abi_equal"))
zero = [s for s, e in ready if e == 0]
print(f"\ncommittable under target flags: {len(ready)}, "
      f"of which {len(zero)} are zero-edit")
for s, e in ready:
    print(f"  {s}" + ("" if e == 0 else f"   ({e} edit(s) - NOT a pure rename)"))

import pathlib as _pl
_pl.Path("/tmp/pbsd_targetflags.txt").write_text(
    f"  under target flags: IR {d['ir_equal']}/{d['ir_ran']}  "
    f"ABI {d.get('abi_equal', 0)}  committable {len(ready)}\n")
PY2

echo "== why 25 ports export a mangled name"
# Ask with the flags the oracle actually uses, on the actual file.
#
# Two explanations were proposed for the fmaximum/fminimum family coming
# out IR-equal and ABI-unequal, and both were wrong the same way - each
# answered a question the oracle does not ask.
#
#   1. "the __ISO_C_VISIBLE >= 2023 guard in the tree's math.h."
#      Tested: the macro is 2023 under -std=c++23 exactly as under -std=c17.
#   2. "the tree's math.h is on -idirafter, so the host's wins." A bare
#      probe of `clang++ -idirafter <msun/src>` agreed - and the oracle also
#      passes -I{src.parent}, and s_fmaximum.c sits in the SAME DIRECTORY as
#      math.h. The tree's header was reachable the whole time. That probe
#      was describing a command line nobody runs, which is exactly the
#      mistake it was added to prevent.
#
# So no more probes of imagined command lines. This runs the oracle's own
# oracle_include_flags() and target flags over the real source and its real
# staged port, and prints the symbol tables both sides actually define,
# plus whether <math.h> under those same flags declares the name.
#
# Diagnostic only; nothing gates on it.
#
# It runs HERE, after the pass phases, and not at the top of the job.
# docs/migration/clang_port/staged/ is generated and .gitignore'd, so on
# a fresh checkout it is empty until run_todo_passes.py stages into it -
# and the first version of this ran before any of that and reported
# "no staged port" for all three files. True, and useless. By this point
# the target-flag phase has staged lib/msun with the same safe tier the
# 173/174 numbers came from, so the ports being examined are the ports
# that were measured.
PROBE_OUT=/tmp/pbsd_probe.txt
( cd "$ROOT/tools" && python3 -m pbsd_passes.why_mangled ) \
    2>&1 | tee "$PROBE_OUT" || true

echo
echo "== SUMMARY"
# The one block worth reading. Everything above is evidence for it.
#
# One file per phase, catted one at a time. `cat a b c` returns non-zero if
# any one of them is missing, so a single unfinished phase used to print the
# "no summary recorded" line underneath the two lines that had in fact been
# recorded. Now a missing file names the phase that did not finish, and the
# blanket line only appears when nothing at all was recorded.
SUMMARY_ANY=0
for f in ratchet:"lib/msun floors" \
         targetflags:"lib/msun under target flags" \
         libc:"lib/libc"; do
    name="${f%%:*}"; label="${f#*:}"
    if [ -s "/tmp/pbsd_${name}.txt" ]; then
        cat "/tmp/pbsd_${name}.txt"
        SUMMARY_ANY=1
    else
        echo "  ${label}: phase did not finish, nothing recorded"
    fi
done
[ "$SUMMARY_ANY" = 1 ] || \
    echo "  (no summary recorded - every phase above failed to finish)"
echo "  why 25 ports export a mangled name (last lines):"
tail -14 /tmp/pbsd_probe.txt 2>/dev/null | sed 's/^/    /' || \
    echo "    (not probed)"
echo "  committed ports: $(find "$ROOT/hbsd/src/lib/msun" -name '*.cpp' | wc -l | tr -d ' ')"
