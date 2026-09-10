#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A FuSeBMC-architecture engine: bounded model checking seeds a fuzzer.

WHAT FuSeBMC IS, AND WHAT THIS IS

FuSeBMC (Alshmrany et al., SV-COMP) is a hybrid: it runs a bounded model
checker to produce counterexample inputs, hands those to a fuzzer as
"smart seeds", and feeds coverage the fuzzer discovers back as new
goals. The two halves cover each other's blind spots -- BMC reasons
about all paths to a shallow depth, fuzzing reaches deep paths it can
stumble into but proves nothing.

The published FuSeBMC is ESBMC + Map2Check + AFL behind a Python driver,
distributed from GitHub. This container's network policy scopes GitHub
to one owner, and ESBMC is in no distribution's package set, so neither
is reachable here. What IS here is CBMC 5.95.1 with goto-cc, and AFL++
4.09c from the archive -- which is the same architecture with a
different BMC engine.

So this is not FuSeBMC. It is FuSeBMC's method, built on the model
checker this tree already uses, and it is named for the method.

WHAT IT PROVES

Nothing. Say that first and keep saying it.

CBMC with --unwind K and --unwinding-assertions can PROVE a bounded
property (cbmc_driver.py's vocabulary, and this reuses it). Fuzzing
cannot: a fuzzer that finds nothing has told you that it found nothing.
The value is asymmetric and worth having anyway -- a crash is a
certainty, and the crashes a fuzzer finds are ones the analyser and the
model checker both missed, because it runs the code.

The status vocabulary is never merged:

  CRASH       the compiled function died on a concrete input, which is
              recorded. ASan/UBSan are on, so this includes reads and
              writes CBMC models and clang's analyser guesses at.
  CLEAN       the fuzzer ran its full budget and found nothing. NOT a
              proof, and never reported as one.
  NOSEED      CBMC could not be run on this function, so the fuzzer got
              no smart seed and was not started. The BMC half is what
              makes this different from blind fuzzing; without it this
              is not the method.
  ERROR       the function could not be compiled into a runnable
              harness. This is the common case and the honest number.
  NOFUNC      not in this translation unit.
  TIMEOUT     the budget expired mid-run with the corpus not drained.

WHY ERROR IS THE COMMON CASE, AND WHY THAT IS THE MEASUREMENT

cbmc_driver.py explains it for CBMC: hbsd/src is FreeBSD source, its
headers collide with glibc's on a Linux host, and most translation units
cannot be compiled standalone here at all. This engine needs strictly
more than CBMC does -- CBMC needs to PARSE the unit, this needs to
COMPILE, LINK and RUN it -- so its reach is a subset of CBMC's reach,
which is itself 749 of 4737 units.

That ratio is the result, not an embarrassment to be hidden. A tool that
reported a number without saying which functions it never reached would
be worse than useless here.

THE HARNESS, AND ITS LIMIT

A fuzzer needs an entry point that turns bytes into arguments. This
generates one for functions whose parameters are all scalars -- the same
SCALAR class cbmc_driver.py allows by default -- by reading sizeof(T)
bytes per parameter from stdin:

    int main(void) {
        uint8_t buf[N];
        if (fread(buf, 1, sizeof buf, stdin) != sizeof buf) return 0;
        (void)F(*(int32_t *)(buf + 0), *(uint64_t *)(buf + 4));
        return 0;
    }

A pointer parameter is NOT synthesised. cbmc_driver.py runs those under
an explicit stated precondition (--min-null-tree-depth) and records the
assumption; there is no honest equivalent for a fuzzer, which would
either pass NULL (reporting the absence of a caller's contract as a
crash) or a buffer of an invented size (reporting the invention). Those
functions come back ERROR with the reason, which is true.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# Scalars a harness can synthesise by reading bytes. Anything else is a
# reason to stop, not a reason to guess.
SCALAR = {
    "char": 1, "signed char": 1, "unsigned char": 1, "_Bool": 1,
    "short": 2, "unsigned short": 2, "short int": 2,
    "int": 4, "unsigned": 4, "unsigned int": 4, "signed int": 4,
    "long": 8, "unsigned long": 8, "long int": 8,
    "long long": 8, "unsigned long long": 8,
    "int8_t": 1, "uint8_t": 1, "int16_t": 2, "uint16_t": 2,
    "int32_t": 4, "uint32_t": 4, "int64_t": 8, "uint64_t": 8,
    "size_t": 8, "ssize_t": 8, "off_t": 8, "intmax_t": 8, "uintmax_t": 8,
}
CTYPE = {1: "uint8_t", 2: "uint16_t", 4: "uint32_t", 8: "uint64_t"}

# The return type sits on its own line in KNF -- `int\nffs(int mask)\n{'
# -- so the character class before the name has to admit a newline. It did
# not, and ffs(), the first function cbmc_driver.py was ever run on, came
# back as if it had a pointer parameter.
PROTO_RE_TMPL = (
    r'(?:^|\n)(?P<ret>[A-Za-z_][\w \t*\n]*?)\b{fn}\s*\((?P<args>[^;{{]*)\)'
    r'\s*\n?\s*\{{'
)


def parse_params(text: str, fn: str):
    """Parameter (type, name) list for fn.

    Returns None when no definition was found at all, and the string
    reason when one was found but cannot be harnessed -- the caller
    reports those differently, because "this file does not define it"
    and "this takes a pointer" are not the same answer.
    """
    m = re.search(PROTO_RE_TMPL.format(fn=re.escape(fn)), text)
    if not m:
        return None
    args = m.group("args").strip()
    if args in ("", "void"):
        return []
    out = []
    depth = 0
    cur = []
    for ch in args:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur))
            cur = []
            continue
        cur.append(ch)
    out.append("".join(cur))
    params = []
    for a in out:
        a = a.strip()
        if not a:
            return "could not split the parameter list"

        if "*" in a or "[" in a or "..." in a:
            return ("a pointer, array or varargs parameter (%s) cannot be "
                    "synthesised without inventing its object" % a)
        a = re.sub(r'\b(const|volatile|register)\b', ' ', a).strip()
        toks = a.split()
        if len(toks) < 2:
            # `int' with no name, or a typedef this does not know
            ty, nm = " ".join(toks), "a%d" % len(params)
        else:
            ty, nm = " ".join(toks[:-1]), toks[-1]
        ty = re.sub(r'\s+', ' ', ty).strip()
        if ty not in SCALAR:
            return "parameter type %r is not a known scalar" % ty
        params.append((ty, nm))
    return params


def layout(params):
    """[(offset, width, type, name)] and the total byte count.

    The harness reads a parameter from this offset and the seed packer
    writes it to the same one. They were separate before, and a
    counterexample of INT_MIN reached the target as a zero byte -- which
    is the difference between FuSeBMC's "smart seed" and a random one.
    """
    off, out = 0, []
    for i, (ty, nm) in enumerate(params):
        w = SCALAR[ty]
        off += (-off) % w                       # natural alignment
        out.append((off, w, ty, nm))
        off += w
    return out, max(off, 1)


def return_type(text: str, fn: str) -> str:
    m = re.search(PROTO_RE_TMPL.format(fn=re.escape(fn)), text)
    if not m:
        return "int"
    r = re.sub(r'\s+', ' ', m.group("ret")).strip()
    r = re.sub(r'\b(static|inline|__inline|extern|__pure|__const)\b', ' ', r)
    return re.sub(r'\s+', ' ', r).strip() or "int"


def build_harness(src: Path, fn: str, params, ret: str = "int") -> str:
    """C source for a stdin-driven entry point calling fn."""
    slots, total = layout(params)
    calls, decls = [], []
    for i, (off, w, ty, _nm) in enumerate(slots):
        decls.append("\t%s v%d;\n\tmemcpy(&v%d, buf + %d, %d);"
                     % (ty, i, i, off, w))
        calls.append("v%d" % i)
    return (
        '#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n\n'
        '/* Generated by tools/verify/fusebmc.py -- do not edit. */\n'
        'extern %s %s();\n'
        '/*\n'
        ' * The result goes into a volatile sink. Discarded, it lets the\n'
        ' * optimiser see the definition is pure -- both files are in one\n'
        ' * translation unit here -- and delete the call outright, which\n'
        ' * is a full fuzzing budget spent on nothing.\n'
        ' */\n'
        '%s\n'
        'int main(void)\n{\n'
        '\tuint8_t buf[%d];\n\n'
        '\tif (fread(buf, 1, sizeof buf, stdin) != sizeof buf)\n'
        '\t\treturn (0);\n%s\n'
        '\t%s%s(%s);\n'
        '\treturn (0);\n}\n'
        % (ret, fn,
           "" if ret == "void" else "static volatile %s sink;" % ret,
           total, "\n".join(decls),
           "" if ret == "void" else "sink = ", fn, ", ".join(calls))
    )


def cbmc_seed(src: Path, fn: str, params, nbytes: int, timeout: int):
    """A seed from CBMC: its counterexample values if it has one, else None.

    This is the half that makes it FuSeBMC's method rather than fuzzing.
    A seed that lands on a path CBMC already reached is worth more than a
    random one, and when CBMC PROVED the function the fuzzer is being
    asked to look somewhere BMC has already closed -- which is exactly
    when its budget should go elsewhere.
    """
    cmd = ["cbmc", str(src), "--function", fn, "--unwind", "4",
           "--bounds-check", "--pointer-check", "--div-by-zero-check",
           "--signed-overflow-check", "--trace"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=timeout)
    except (subprocess.TimeoutExpired, OSError):
        return None, "cbmc did not run"
    out = (p.stdout or "") + (p.stderr or "")
    if "VERIFICATION FAILED" in out:
        # CBMC prints `  j=-2147483648 (10000000 ...)' for each input.
        # Pack each named value at the width and offset the harness reads
        # it from -- truncating to a byte, as this first did, discards
        # exactly the extreme values a counterexample is made of.
        seen = {m.group(1): int(m.group(2)) for m in re.finditer(
            r'^\s*([A-Za-z_]\w*)=(-?\d+)\s*\(', out, re.M)}
        buf = bytearray(nbytes)
        slots, _ = layout(params)
        got = False
        for off, w, _ty, nm in slots:
            if nm in seen:
                buf[off:off + w] = (seen[nm] & ((1 << (8 * w)) - 1)) \
                    .to_bytes(w, "little")
                got = True
        if got:
            return bytes(buf), "counterexample"
        return bytes(buf), "counterexample-unmapped"
    if "VERIFICATION SUCCESSFUL" in out:
        return bytes(nbytes), "proved-within-bound"
    return None, _tail(out)


def _tail(s: str, n: int = 300) -> str:
    return s.strip()[-n:]


def fuzz_one(src: Path, fn: str, budget: int, workdir: Path,
             cbmc_timeout: int) -> dict:
    rec = {"file": str(src.relative_to(SRC)), "function": fn}
    try:
        text = src.read_text(errors="replace")
    except OSError as e:
        return {**rec, "status": "ERROR", "detail": str(e)}

    params = parse_params(text, fn)
    if params is None:
        return {**rec, "status": "NOFUNC",
                "detail": "no definition of %s in this translation unit" % fn}
    if isinstance(params, str):
        return {**rec, "status": "ERROR", "detail": params}

    harness = build_harness(src, fn, params, return_type(text, fn))
    nbytes = int(re.search(r'uint8_t buf\[(\d+)\]', harness).group(1))

    work = workdir / ("%s_%s" % (src.stem, fn))
    shutil.rmtree(work, ignore_errors=True)
    (work / "in").mkdir(parents=True)
    (work / "harness.c").write_text(harness)

    t_start = time.time()
    seed, why = cbmc_seed(src, fn, params, nbytes, cbmc_timeout)
    if seed is None:
        return {**rec, "status": "NOSEED", "detail": why}
    (work / "in" / "seed").write_bytes(seed)

    cc = shutil.which("afl-clang-fast") or shutil.which("afl-cc")
    if cc is None:
        return {**rec, "status": "ERROR", "detail": "no afl compiler"}
    binp = work / "target"
    env = {**os.environ, "AFL_QUIET": "1", "AFL_LLVM_INSTRUMENT": "CLASSIC"}
    # -fno-builtin is not optional and the reason is worth stating.
    #
    # abs() is a compiler builtin. Without it, clang recognised the call
    # in the harness, lowered it to llvm.abs with is_int_min_poison=false,
    # and NEVER CALLED the definition in abs.c -- so the engine ran a
    # full fuzzing budget against the compiler's implementation and
    # reported CLEAN on a function it had not executed. The same is true
    # of memcpy, strlen, memset and every other libc name clang knows.
    # A harness without -fno-builtin is testing the compiler.
    cp = subprocess.run(
        [cc, "-g", "-O1", "-fno-builtin", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(work / "harness.c"), str(src),
         "-o", str(binp)],
        capture_output=True, text=True, env=env)
    if cp.returncode != 0:
        return {**rec, "status": "ERROR",
                "detail": _tail((cp.stdout or "") + (cp.stderr or ""))}

    # PHASE: replay the BMC seed before fuzzing.
    #
    # This is the point of the hybrid and it is easy to miss. When CBMC
    # hands back a counterexample, that counterexample IS an input that
    # violates a property -- so running it is the cheapest possible test,
    # and if it dies there is nothing left to fuzz for. AFL++ also
    # refuses to start at all when every seed crashes ("We need at least
    # one valid input seed that does not crash!"), which the first
    # version of this read as CLEAN. A fuzzer that would not start and a
    # fuzzer that found nothing are not the same answer.
    SAN = {"UBSAN_OPTIONS": "halt_on_error=1:abort_on_error=1:print_stacktrace=1",
           "ASAN_OPTIONS": "abort_on_error=1:symbolize=0:detect_leaks=0"}
    rp = subprocess.run([str(binp)], input=seed, capture_output=True,
                        timeout=30, env={**os.environ, **SAN})
    if rp.returncode != 0:
        return {**rec, "status": "CRASH", "seed": why, "found_by": "bmc-seed",
                "inputs": [seed.hex()],
                "detail": _tail((rp.stderr or b"").decode("utf-8", "replace")),
                "elapsed": time.time() - t_start}

    # A second, all-zero seed so the corpus survives one bad input.
    (work / "in" / "zero").write_bytes(bytes(nbytes))

    out = work / "out"
    # UBSan exits 1 by default, and AFL detects crashes by SIGNAL -- so
    # without abort_on_error a UBSan report is invisible to the fuzzer.
    env2 = {**env, **SAN, "AFL_SKIP_CPUFREQ": "1", "AFL_NO_AFFINITY": "1",
            "AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES": "1",
            "AFL_BENCH_UNTIL_CRASH": "1"}
    t0 = time.time()
    try:
        ap_ = subprocess.run(
            ["afl-fuzz", "-i", str(work / "in"), "-o", str(out),
             "-V", str(budget), "-m", "none", "--", str(binp)],
            capture_output=True, text=True, timeout=budget + 60, env=env2)
        aflout = (ap_.stdout or "") + (ap_.stderr or "")
        (work / "afl.log").write_text(aflout)
        # A fuzzer that refused to start is not a fuzzer that found
        # nothing, and the two must never come back as the same word.
        if "PROGRAM ABORT" in aflout or "All test cases processed." \
                not in aflout:
            return {**rec, "status": "ERROR",
                    "detail": "afl-fuzz did not start: " + _tail(aflout)}
    except subprocess.TimeoutExpired:
        return {**rec, "status": "TIMEOUT", "elapsed": time.time() - t0}
    except OSError as e:
        return {**rec, "status": "ERROR", "detail": str(e)}

    crashes = sorted((out / "default" / "crashes").glob("id:*")) \
        if (out / "default" / "crashes").is_dir() else []
    if crashes:
        return {**rec, "status": "CRASH", "seed": why,
                "inputs": [c.read_bytes().hex() for c in crashes[:4]],
                "elapsed": time.time() - t0}
    return {**rec, "status": "CLEAN", "seed": why,
            "elapsed": time.time() - t0,
            "note": "found nothing in this budget; not a proof"}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", action="append", default=[],
                    help="path under hbsd/src, repeatable")
    ap.add_argument("--function", action="append", default=[])
    ap.add_argument("--budget", type=int, default=30,
                    help="seconds of fuzzing per function")
    ap.add_argument("--cbmc-timeout", type=int, default=30)
    ap.add_argument("--out", default="fusebmc_results.jsonl")
    ap.add_argument("--keep", action="store_true",
                    help="keep the harness/corpus tree for inspection")
    args = ap.parse_args()

    if len(args.src) != len(args.function):
        print("--src and --function must pair up", file=sys.stderr)
        return 2

    work = Path(tempfile.mkdtemp(prefix="fusebmc-"))
    if args.keep:
        print("workdir: %s" % work, file=sys.stderr)
    counts: dict[str, int] = {}
    with open(args.out, "w") as fh:
        fh.write(json.dumps({"_meta": "fusebmc", "budget": args.budget}) + "\n")
        for s, f in zip(args.src, args.function):
            r = fuzz_one(SRC / s, f, args.budget, work, args.cbmc_timeout)
            counts[r["status"]] = counts.get(r["status"], 0) + 1
            print("%-9s %s %s%s" % (r["status"], s, f,
                                    ("  " + r["detail"][:70])
                                    if r.get("detail") else ""))
            fh.write(json.dumps(r) + "\n")
    print("\n" + "  ".join("%s=%d" % kv for kv in sorted(counts.items())),
          file=sys.stderr)
    print("CLEAN is not a proof. ERROR is the measurement of what this "
          "could not reach.", file=sys.stderr)
    if not args.keep:
        shutil.rmtree(work, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
