#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Model checking over every function in hbsd/src, with ESBMC.

THIS DRIVER HAS NEVER BEEN RUN AGAINST A REAL ESBMC BINARY.

Say that first and keep saying it. `esbmc` is in no distribution's
package set and is not on PyPI - fusebmc.py's docstring recorded that,
and it was re-checked when this file was written:

    # apt-cache policy esbmc      -> no candidate, no such package
    # pip download esbmc          -> No matching distribution found

So every flag below comes from ESBMC's DOCUMENTED command line and every
output pattern from its documented format. Neither has been confirmed
against the tool. What HAS been executed is the parsing: mock/esbmc is a
shell script that emits ESBMC's output shapes, and test_esbmc_driver.py
runs this driver against it and asserts every status mapping. That makes
the parser landable. It does not make the flags right.

The three flags most likely to be wrong are named in FLAG_CONFIDENCE
below and printed by --selftest, which also probes `esbmc --help` for
every flag this driver would pass. Run that FIRST on any machine that
has the real binary. A flag ESBMC does not recognise is a loud error; a
flag that silently disables a check is the failure this whole repository
is written against, and the check polarity is where it hides:

    CBMC:   checks are OFF  by default, --bounds-check turns one ON.
    ESBMC:  bounds, pointer and div-by-zero are ON by default, and
            --no-bounds-check turns one OFF. Overflow, NaN and
            memory-leak are OFF and turned on by name.

The polarity is INVERTED for half the set. tools/pbsd_agent/esbmc_check.py
in this tree passes `--no-bounds-check` unconditionally, which disables
array bounds checking on every run it has ever made. That is the mistake
in its live form, and it is why this driver never passes a --no-* check
flag and asserts as much in check_flags().

WHY ESBMC AT ALL, GIVEN CBMC IS ALREADY HERE

Three things CBMC cannot do, in descending order of what they are worth:

1. K-INDUCTION PROVES WITHOUT A BOUND. cbmc_driver.py's PROVED is a
   proof that the loops closed inside --unwind K; it is a real theorem
   and it is still a theorem about a bound. ESBMC's --k-induction can
   discharge the inductive step, and a property that survives the
   inductive step holds for EVERY iteration count. That is strictly
   stronger, so it gets its own status and is never merged down:

       PROVED-UNBOUNDED   the inductive step or the forward condition
                          closed it. No loop bound is involved.
       PROVED             bounded, exactly cbmc_driver.py's meaning.

   Merging those two would be the same error as merging PROVED and
   BOUNDED, one level up.

2. A DIFFERENT SOLVER AND ENCODING. CBMC bit-blasts to SAT; ESBMC goes
   to SMT (Boolector by default, Z3/CVC/MathSAT/Yices/Bitwuzla
   selectable). Run 33 has 212 of 424 ERRORs being CBMC's SAT back end
   running out of memory. An SMT encoding with bitvector theories does
   not necessarily fall over on the same functions - and will fall over
   on different ones. The point is the disagreement, not a winner.

3. CONCURRENCY. --context-bound N bounds interleavings, and
   --data-races-check / --deadlock-check / --lock-order-check are defect
   classes this harness currently cannot reach AT ALL. Those are opt-in
   here because they are a different question from the UB tier, and
   because the kernel functions worth asking them about are the ones
   classify.py calls POINTER, which this driver does not run by default.

WHAT IT CANNOT DO, AND THE ONE THAT MATTERS MOST

ESBMC CANNOT READ CBMC'S GOTO BINARIES. classify.py's real output is a
tree of .gb files written by goto-cc, and cbmc_driver.py runs CBMC
directly on those. ESBMC's --binary reads ESBMC's own serialised goto
program; the ireps are not the same format and there is no documented
converter. So this driver uses classify.py's output for the SIGNATURE
CLASSES and LINKAGE only, and gets the code from source:

  * --preprocessed DIR is the honest path. A self-contained .i built on
    a FreeBSD host has no header problem to have, and ESBMC parses it
    anywhere. Same remedy cbmc_driver.py's docstring names.
  * failing that, hbsd/src/<path> with includes.py's -I set, which on a
    Linux host will ERROR for most of the tree for exactly the reason
    cbmc_driver.py's PREPROCESSING section gives.

That is a real reduction in reach relative to the CBMC sweep, and it is
recorded rather than worked around: an unreachable translation unit is
ERROR with the compiler's message, never a clean result.

THE STATUS VOCABULARY

cbmc_driver.py's, plus three. None of them is ever merged:

  PROVED-UNBOUNDED  k-induction's inductive step, or the forward
                    condition, closed it. Holds for all iteration
                    counts. STRONGER than PROVED.
  PROVED            every property discharged, loops closed inside the
                    bound, and unwinding assertions were demonstrably
                    active. Bounded proof, cbmc_driver.py's meaning.
  BOUNDED           nothing found within the bound, and that is all.
                    Also where a clean bounded run lands when this
                    driver cannot PROVE that unwinding assertions were
                    on - see UNWIND_ASSERT_POLICY.
  FAILED            a violated property with a counterexample.
  UNKNOWN           ESBMC said VERIFICATION UNKNOWN. It ran, it did not
                    conclude. New here: CBMC has no such answer. Not
                    clean, not a finding.
  TIMEOUT           did not finish inside the wall clock.
  ERROR             could not be parsed, converted, or solved.
  NOFUNC            the named function is not in this translation unit.
  NOTRUN            `esbmc` is not on PATH, so NOTHING WAS CHECKED.

NOTRUN exists because this repository's whole premise is that a scope
nobody checked looks exactly like one that is clean. A missing binary is
the purest form of that, and it is the form most likely to go unnoticed,
because it costs no time and prints no error. So it is a status, it is
written to the JSONL for every task, and a run that produced any of it
exits non-zero. There is no combination of arguments to this driver that
turns an absent ESBMC into a green run.

Results are appended to JSONL as they are produced, so a run is
resumable and a kill loses at most the in-flight functions.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import resource
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

# As if this file sits in tools/verify/, which is where it belongs.
ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"


# ---------------------------------------------------------------------------
# Flags, and exactly how much is known about each one.
# ---------------------------------------------------------------------------
#
# Every flag this driver can pass is listed here with a confidence, and
# --selftest prints the table and probes `esbmc --help` for each entry.
# "high" means ESBMC's documented option list carries the flag with this
# spelling and this meaning. "low" means it is the best reading of the
# documentation and has to be confirmed before a result under it is
# worth anything.
#
#   flag -> (confidence, what it is for)
FLAG_CONFIDENCE: dict[str, tuple[str, str]] = {
    "--function": ("high", "entry point, same role as CBMC's --function"),
    "--unwind": ("high", "loop bound for the bounded mode"),
    "--overflow-check": (
        "high", "signed arithmetic over/underflow. ESBMC's own wording is "
                "'arithmetic over- and underflow check'; the unsigned case "
                "is a SEPARATE flag, so this is CBMC's "
                "--signed-overflow-check and not its --unsigned one."),
    "--unsigned-overflow-check": ("high", "unsigned wrap. Advisory: defined."),
    "--nan-check": ("high", "floating point NaN. Advisory."),
    "--memory-leak-check": ("high", "CBMC's --memory-leak-check"),
    "--ub-shift-check": (
        "medium", "undefined shift. CBMC spells it --undefined-shift-check. "
                  "The ESBMC spelling is believed to be --ub-shift-check; "
                  "confirm before trusting a PROVED that relied on it."),
    "--struct-fields-check": (
        "medium", "over-sized reads of struct fields. No CBMC equivalent, "
                  "so it is off by default here rather than folded into a "
                  "tier whose meaning is set by the CBMC run."),
    "--k-induction": ("high", "the whole reason this driver exists"),
    "--max-k-step": ("high", "ceiling on k for the k-induction family"),
    "--k-step": ("medium", "k increment. Default 1; not passed unless asked."),
    "--incremental-bmc": ("high", "base case + forward condition, growing k"),
    "--falsification": (
        "medium", "incremental base case only - bug hunting, never a proof. "
                  "Believed to be spelled --falsification."),
    "--context-bound": (
        "medium", "bound on context switches per thread. The concurrency "
                  "mode is built on this and it is NOT confirmed."),
    "--data-races-check": ("medium", "concurrency defect class, opt-in"),
    "--deadlock-check": ("medium", "concurrency defect class, opt-in"),
    "--lock-order-check": ("low", "lock acquisition ordering, opt-in"),
    "--unwinding-assertions": (
        "low", "THE MOST IMPORTANT UNCERTAINTY IN THIS FILE. ESBMC is "
               "believed to generate unwinding assertions BY DEFAULT and to "
               "take --no-unwinding-assertions to suppress them, which is "
               "the opposite of CBMC. Whether the positive spelling exists "
               "at all is not known. See UNWIND_ASSERT_POLICY: the default "
               "policy passes NEITHER and refuses to say PROVED without "
               "evidence, so being wrong here costs a weaker verdict "
               "rather than a false one."),
    "--no-unwinding-assertions": (
        "medium", "suppress unwinding assertions. Only reachable via "
                  "--unwind-assert off, which caps the verdict at BOUNDED."),
    "--memlimit": (
        "medium", "ESBMC's own memory ceiling, spelled '2g'. RLIMIT_AS via "
                  "--mem-mb is the authority here because it is enforced by "
                  "the kernel and needs no flag to be right."),
    "--timeout": (
        "low", "ESBMC's own wall clock. The ARGUMENT FORMAT is the "
               "uncertainty - documented as an integer with an s/m/h "
               "suffix, while tools/pbsd_agent/esbmc_check.py passes a bare "
               "integer. Not passed unless --esbmc-timeout is given; "
               "subprocess.run's timeout is the authority and needs no "
               "agreement about formats."),
    "--boolector": ("high", "default solver"),
    "--z3": ("high", "solver"),
    "--cvc": ("medium", "solver. Newer ESBMC may spell it --cvc4/--cvc5."),
    "--mathsat": ("medium", "solver"),
    "--yices": ("medium", "solver"),
    "--bitwuzla": ("medium", "solver"),
    "--64": ("high", "machine word width"),
    "--little-endian": ("high", "byte order"),
    "-I": ("high", "include path, same as a compiler"),
    "-D": ("high", "macro definition, same as a compiler"),
}

# CBMC's UB tier -> ESBMC. The ones with no entry have no equivalent this
# driver is willing to guess at, and they are named in MISSING_CHECKS so
# the gap is in the record rather than in nobody's head.
#
# NOTE THE POLARITY. bounds, pointer and div-by-zero are ESBMC defaults
# and appear here as NOTHING TO PASS. Passing --no-bounds-check would
# turn one off; this driver never does, and check_flags() enforces it.
UB_CHECKS = [
    "--overflow-check",        # CBMC --signed-overflow-check
    "--ub-shift-check",        # CBMC --undefined-shift-check
    "--memory-leak-check",     # CBMC --memory-leak-check
]

ADVISORY_CHECKS = [
    "--unsigned-overflow-check",
    "--nan-check",
]

# On by default in ESBMC. Recorded so a reader of a result knows the UB
# tier really did include them, and so check_flags() can refuse a command
# line that switched one off.
DEFAULT_ON_CHECKS = ["bounds", "pointer", "div-by-zero"]
FORBIDDEN_FLAGS = ("--no-bounds-check", "--no-pointer-check",
                   "--no-div-by-zero-check", "--no-assertions")

# Checks cbmc_driver.py runs that this driver does NOT, because no ESBMC
# flag is known to correspond and inventing one is worse than the gap.
MISSING_CHECKS = {
    "--pointer-overflow-check": (
        "ub", "CBMC checks pointer arithmetic that leaves the object. "
              "ESBMC's --no-pointer-relation-check suppresses a pointer "
              "RELATION check, which is a different property. No confirmed "
              "equivalent, so the ESBMC UB tier is NARROWER than CBMC's "
              "by this one check."),
    "--conversion-check": (
        "advisory", "no known ESBMC equivalent. Advisory anyway, and "
                    "cbmc_driver.py's docstring explains why it is not in "
                    "the UB tier."),
    "--float-overflow-check": (
        "advisory", "no known ESBMC equivalent."),
}


# ---------------------------------------------------------------------------
# Output patterns. Documented ESBMC format; see the module docstring.
# ---------------------------------------------------------------------------

SUCCESS_RE = re.compile(r"^VERIFICATION SUCCESSFUL\s*$", re.M)
FAILED_RE = re.compile(r"^VERIFICATION FAILED\s*$", re.M)
UNKNOWN_RE = re.compile(r"^VERIFICATION UNKNOWN\s*$", re.M)

# The lines that make a k-induction or incremental result UNBOUNDED.
#
#   Solution found by the inductive step (k = 4)
#   Solution found by the forward condition; all states are reachable (k = 3)
#
# Both mean the loop is fully accounted for: the inductive step closes
# the property for every iteration count, and the forward condition says
# the state space was exhausted at k. Either one is a proof with no bound
# in it. The base case is NOT one of them - it is BMC at depth k and
# nothing more, so it is matched separately and deliberately.
INDUCTIVE_RE = re.compile(
    r"Solution found by the (inductive step|forward condition)", re.I)
BASECASE_RE = re.compile(r"Solution found by the base case", re.I)
K_RE = re.compile(r"\(k\s*=\s*(\d+)\)")

# ESBMC prints one of these per violated property:
#
#   Violated property:
#     file foo.c line 7 column 5 function f
#     arithmetic overflow on add
#     !overflow("+", a, b)
#
# The middle line is the human description and the last is the encoded
# expression. Formatted below into CBMC's "line N <desc>" shape on
# purpose, so report.py's bucketing - which does
# re.match(r"line (\d+) (.*)", desc) - reads an ESBMC record without
# knowing it is one.
VIOLATED_RE = re.compile(
    r"^Violated property:\s*$\n"
    r"^\s*file\s+(?P<file>\S+)\s+line\s+(?P<line>\d+)"
    r"(?:\s+column\s+\d+)?(?:\s+function\s+(?P<fn>\S+))?\s*$\n"
    r"^\s*(?P<desc>\S.*?)\s*$"
    r"(?:\n^\s*(?P<expr>\S.*?)\s*$)?",
    re.M)

# "Generated 12 VCC(s), 7 remaining after simplification"
VCC_RE = re.compile(r"Generated\s+(\d+)\s+VCC\(s\),\s+(\d+)\s+remaining")

# Same class name as CBMC's, and it means the same thing: the loop wanted
# more iterations than it was given.
UNWIND_RE = re.compile(r"\bunwinding assertion\b", re.I)

# ESBMC declining to answer, in the shapes it is documented to use.
NOFUNC_RE = re.compile(
    r"(?:function|symbol)\s+[`'\"]?(?P<fn>[A-Za-z_]\w*)[`'\"]?\s+"
    r"(?:is\s+)?not\s+found|not\s+found.{0,20}function", re.I)
PARSE_ERR_RE = re.compile(
    r"^\s*(?:PARSING ERROR|ERROR:\s*PARSING ERROR|CONVERSION ERROR)\s*$",
    re.M | re.I)

# The lines that say WHY, hoisted to the head of the detail window. Same
# job as cbmc_driver.py's _WHY_RE and the same reason: report.py reads
# the first few lines of `detail' looking for a reason, and an arbitrary
# 600-character truncation window does not contain one.
_WHY_RE = re.compile(
    r"^(?:.*?\bPARSING ERROR\b.*|.*?\bCONVERSION ERROR\b.*|"
    r"\s*(?:SAT checker ran )?[Oo]ut of memory\s*|"
    r".*?\bUnable to (?:open|parse)\b.*|"
    r".*?\bunsupported\b.*|.*?\bfatal error:.*|.*?\berror:.*|"
    r".*?\bNo solver\b.*|.*?\bexception\b.*)$",
    re.M | re.I)


def esbmc_bin() -> str:
    """Whatever ESBMC this process should run.

    PBSD_ESBMC exists so the test suite can point the driver at
    mock/esbmc without the mock having to be on PATH, and so a machine
    with a hand-built ESBMC does not have to shadow anything.
    """
    env = os.environ.get("PBSD_ESBMC")
    if env:
        return env
    return shutil.which("esbmc") or "esbmc"


def esbmc_available() -> bool:
    """Is there something to run at all?

    Deliberately NOT a version probe. An ESBMC that is present but too
    old fails loudly on an unrecognised flag, which is a good failure. An
    ESBMC that is absent fails silently and for free, which is the one
    this repository cares about, so it is the one that is tested for.
    """
    b = esbmc_bin()
    p = Path(b)
    if p.is_absolute() or os.sep in b:
        return p.is_file() and os.access(b, os.X_OK)
    return shutil.which(b) is not None


# ---------------------------------------------------------------------------
# Unwinding assertions, and why the default policy is the timid one.
# ---------------------------------------------------------------------------
#
# CBMC needs --unwinding-assertions to tell you the bound was too small;
# without it a clean run is meaningless and cbmc_driver.py always passes
# it. ESBMC is believed to have them ON by default, with
# --no-unwinding-assertions to suppress. "Believed" is the problem: if
# that is wrong, and this driver passes nothing, then a loop that wanted
# more than K iterations comes back VERIFICATION SUCCESSFUL and calling
# that PROVED would be a false theorem.
#
# So the default policy is `auto', and under `auto' a clean bounded run is
# BOUNDED unless the output carries positive evidence that unwinding
# assertions were generated - an unwinding-assertion property mentioned
# anywhere, or ESBMC saying it unwound a loop to the limit. Being wrong
# about the default then costs a WEAKER verdict, never a false one.
#
#   auto  pass nothing. PROVED only with evidence; otherwise BOUNDED.
#   on    pass --unwinding-assertions. If that spelling does not exist
#         ESBMC errors out, which is loud, and the record is ERROR.
#   off   pass --no-unwinding-assertions. Caps the verdict at BOUNDED
#         by construction; there is nothing to detect.
UNWIND_ASSERT_POLICY = ("auto", "on", "off")

# Evidence, in the output, that the bound was actually being policed.
UNWIND_EVIDENCE_RE = re.compile(
    r"unwinding assertion|unwind(?:ing)? (?:loop|bound).{0,40}\blimit\b|"
    r"aborting path on unwind", re.I)


def check_flags(cmd: list[str]) -> None:
    """Refuse a command line that switched off a check the tier claims.

    This is the one thing in the file worth an assertion rather than a
    comment. The UB tier's meaning is 'bounds, pointer, div-by-zero,
    overflow, shift and leaks were all checked'. Three of those six are
    ESBMC defaults, which means they are enabled by the ABSENCE of a
    flag - and an absence is not something a reader of a result can
    see. A --extra that happened to carry --no-pointer-check would
    produce PROVED records that mean something narrower than PROVED
    says, with nothing anywhere to indicate it.

    tools/pbsd_agent/esbmc_check.py is that mistake, already in the
    tree: it passes --no-bounds-check on every run.
    """
    bad = [f for f in cmd if f.split("=")[0] in FORBIDDEN_FLAGS]
    if bad:
        raise ValueError(
            f"refusing to run: {' '.join(bad)} would disable a check this "
            f"tier claims to have made. ESBMC enables "
            f"{', '.join(DEFAULT_ON_CHECKS)} by DEFAULT; a --no-* flag is "
            f"how they get lost silently.")


def _mem_capped(mb: int):
    """An RLIMIT_AS for the child, or None to leave it uncapped.

    Copied wholesale from cbmc_driver.py, including the reasoning, because
    the hazard is the solver's and not CBMC's: a model checker that can
    exhaust a 16GB runner inside its wall-clock timeout takes the whole
    shard with it, and the partial .jsonl goes too. RLIMIT_AS turns that
    into one function's ERROR.

    ESBMC also has --memlimit, which is not used as the authority here:
    a kernel-enforced rlimit needs no agreement about an argument format,
    and --memlimit's is one of the things this driver is unsure of.

    ProcessPoolExecutor means each task already runs in its own process,
    so preexec_fn here is safe.
    """
    if mb <= 0:
        return None

    def _limit() -> None:
        lim = mb * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (lim, lim))

    return _limit


def mode_flags(task: dict) -> list[str]:
    """The flags that make one mode that mode.

    The modes are not interchangeable and their results are not
    comparable, which is why `mode' is in every record:

      bounded      --unwind K. Directly comparable to a cbmc_driver.py
                   run at the same K, which is the entire point of
                   having it: A/B on the same functions with a different
                   solver and a different encoding.
      kinduction   --k-induction --max-k-step N. The only mode that can
                   return PROVED-UNBOUNDED by the inductive step.
      incremental  --incremental-bmc --max-k-step N. Grows k until it
                   finds a bug or the forward condition closes the state
                   space. Can also reach PROVED-UNBOUNDED, by the
                   forward condition rather than the inductive step.
      falsification  base case only. Bug hunting. CANNOT prove anything
                   and its clean answer is BOUNDED, never PROVED.
      concurrency  bounded plus --context-bound. The defect classes are
                   opt-in via --concurrency-checks.
    """
    mode = task["mode"]
    f: list[str] = []
    if mode == "bounded":
        f += ["--unwind", str(task["unwind"])]
    elif mode == "kinduction":
        f += ["--k-induction", "--max-k-step", str(task["max_k"])]
        if task.get("k_step"):
            f += ["--k-step", str(task["k_step"])]
    elif mode == "incremental":
        f += ["--incremental-bmc", "--max-k-step", str(task["max_k"])]
    elif mode == "falsification":
        f += ["--falsification", "--max-k-step", str(task["max_k"])]
    elif mode == "concurrency":
        f += ["--unwind", str(task["unwind"]),
              "--context-bound", str(task["context_bound"])]
        if task.get("concurrency_checks"):
            f += ["--data-races-check", "--deadlock-check"]
    else:
        raise ValueError(f"unknown mode {mode!r}")

    pol = task.get("unwind_assert", "auto")
    if pol == "on":
        f.append("--unwinding-assertions")
    elif pol == "off":
        f.append("--no-unwinding-assertions")
    return f


SOLVER_FLAG = {
    "default": None,          # let ESBMC pick; documented default Boolector
    "boolector": "--boolector",
    "z3": "--z3",
    "cvc": "--cvc",
    "mathsat": "--mathsat",
    "yices": "--yices",
    "bitwuzla": "--bitwuzla",
}


def build_cmd(task: dict) -> list[str]:
    """The whole ESBMC command line for one (source, function) pair."""
    checks = ADVISORY_CHECKS if task["tier"] == "advisory" else UB_CHECKS
    cmd = [esbmc_bin(), task["src"], "--function", task["function"]]
    cmd += list(checks)
    cmd += mode_flags(task)

    sf = SOLVER_FLAG.get(task.get("solver", "default"))
    if sf:
        cmd.append(sf)
    if task.get("memlimit"):
        cmd += ["--memlimit", task["memlimit"]]
    if task.get("esbmc_timeout"):
        # Documented as an integer with an s/m/h suffix. Only reachable by
        # asking for it, because the format is the uncertain part and
        # subprocess.run's timeout does the job with no format at all.
        cmd += ["--timeout", f"{int(task['esbmc_timeout'])}s"]
    cmd += list(task.get("cflags", []))
    cmd += list(task.get("extra", []))
    check_flags(cmd)
    return cmd


def classify_output(task: dict, out: str, rc: int) -> tuple[str, dict]:
    """ESBMC's output -> one status, plus the fields that justify it.

    Order matters and is not arbitrary. A violated property that is an
    unwinding assertion is not a defect - it is the bound being too
    small, exactly as in cbmc_driver.py - so the split between real
    failures and unwinding failures happens before the verdict, not
    after it.

    `rc' is taken and deliberately NOT used. ESBMC's exit codes are one
    of the things this driver was unable to confirm: 0 for SUCCESSFUL
    and non-zero otherwise is the obvious reading, and cbmc_driver.py
    was bitten by exactly this with CBMC's 0-or-10. The verdict line is
    unambiguous and is in the documented output, so it decides, and the
    exit code is left in the signature for whoever measures what it
    really does on a real binary.
    """
    extra: dict = {}

    props = []
    for m in VIOLATED_RE.finditer(out):
        desc = (m.group("desc") or "").strip()
        props.append({
            # CBMC's shape, so report.py buckets an ESBMC record without
            # being taught about ESBMC.
            "desc": f"line {m.group('line')} {desc}",
            "expr": (m.group("expr") or "").strip()[:200],
            "file": m.group("file"),
            "line": int(m.group("line")),
        })
    real = [p for p in props if not UNWIND_RE.search(p["desc"])]
    unwound = [p for p in props if UNWIND_RE.search(p["desc"])]

    vcc = VCC_RE.search(out)
    if vcc:
        extra["vcc_generated"] = int(vcc.group(1))
        extra["vcc_remaining"] = int(vcc.group(2))

    ind = INDUCTIVE_RE.search(out)
    base = BASECASE_RE.search(out)
    km = K_RE.search(out[ind.start():] if ind else (out[base.start():] if base else ""))
    if km:
        extra["k"] = int(km.group(1))
    if ind:
        extra["closed_by"] = ind.group(1).lower().replace(" ", "-")
    elif base:
        extra["closed_by"] = "base-case"

    # Did the tool get far enough to answer at all?
    said_ok = bool(SUCCESS_RE.search(out))
    said_bad = bool(FAILED_RE.search(out))
    said_unk = bool(UNKNOWN_RE.search(out))

    if not (said_ok or said_bad or said_unk):
        low = out.lower()
        if NOFUNC_RE.search(out) and task["function"].lower() in low:
            return "NOFUNC", extra
        return "ERROR", extra

    if said_bad:
        if real:
            extra["failures"] = [
                {"desc": p["desc"], "expr": p["expr"]} for p in real[:12]]
            extra["unwind_hit"] = bool(unwound)
            return "FAILED", extra
        if unwound:
            # The bound was too small and nothing else went wrong. Same
            # meaning as cbmc_driver.py's BOUNDED, reached from ESBMC's
            # opposite direction: CBMC reports it as a failed property
            # inside a SUCCESSFUL-looking run, ESBMC as VERIFICATION
            # FAILED whose only violated property is the unwinding one.
            extra["unwind_hit"] = True
            return "BOUNDED", extra
        # FAILED with no property this driver could parse. Not a finding
        # and not clean: it is a record nobody can act on, which is an
        # ERROR by the same argument that made _tail() keep why-lines.
        return "ERROR", extra

    if said_unk:
        extra["unwind_hit"] = bool(unwound)
        return "UNKNOWN", extra

    # VERIFICATION SUCCESSFUL from here down.
    mode = task["mode"]
    if ind:
        # The inductive step or the forward condition closed it. No loop
        # bound is involved in the claim.
        return "PROVED-UNBOUNDED", extra
    if mode == "falsification":
        # Base case only. It looked for a bug and did not find one at
        # this depth. That is not a proof and is never recorded as one.
        return "BOUNDED", extra
    if mode in ("kinduction", "incremental"):
        # Clean, but nothing said the induction closed. Either it ran out
        # of k, or this driver failed to recognise the line that says it
        # did - and the second possibility is exactly why this is not
        # PROVED-UNBOUNDED on the strength of the mode alone.
        return "BOUNDED", extra
    if unwound:
        extra["unwind_hit"] = True
        return "BOUNDED", extra

    # Bounded mode, clean, no unwinding assertion violated. Whether that
    # is a PROOF depends entirely on whether unwinding assertions were
    # being generated, and under the default policy this driver does not
    # know. See UNWIND_ASSERT_POLICY.
    pol = task.get("unwind_assert", "auto")
    if pol == "on" or (pol == "auto" and UNWIND_EVIDENCE_RE.search(out)):
        return "PROVED", extra
    extra["unwind_assert_unconfirmed"] = True
    return "BOUNDED", extra


def verify_one(task: dict) -> dict:
    """Run ESBMC on one (translation unit, function) pair."""
    if not esbmc_available():
        # Never clean. See the module docstring.
        return _rec(task, "NOTRUN",
                    detail=f"esbmc not found on PATH (looked for "
                           f"{esbmc_bin()!r}); NOTHING was checked",
                    elapsed=0.0)
    try:
        cmd = build_cmd(task)
    except ValueError as e:
        return _rec(task, "ERROR", detail=str(e), elapsed=0.0)

    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=task["timeout"],
                           preexec_fn=_mem_capped(task.get("mem_mb", 0)))
        out = (p.stdout or "") + "\n" + (p.stderr or "")
        rc = p.returncode
    except subprocess.TimeoutExpired:
        return _rec(task, "TIMEOUT", elapsed=task["timeout"])
    except OSError as e:
        return _rec(task, "ERROR", detail=str(e), elapsed=time.time() - t0)

    elapsed = time.time() - t0
    status, extra = classify_output(task, out, rc)
    if status in ("ERROR", "NOFUNC", "UNKNOWN"):
        extra.setdefault("detail", _tail(out))
    return _rec(task, status, elapsed=elapsed, **extra)


def _tail(s: str, n: int = 600) -> str:
    """The part of ESBMC's output worth keeping.

    Same construction as cbmc_driver.py's, for the same reason: the last
    n characters is right for a counterexample, whose interesting part is
    at the end, and throws away the only useful line of a parse error,
    whose interesting part is at the top followed by a page of clang
    diagnostics. So the why-lines come first, then the tail.

    A record nobody can read is the same as no record: it cannot be told
    from a function nobody checked.
    """
    s = s.strip()
    why: list[str] = []
    for m in _WHY_RE.finditer(s):
        line = m.group(0).strip()
        if line and line not in why:
            why.append(line)
        if len(why) == 4:
            break
    head = "\n".join(why)
    if head:
        room = n - len(head) - 1
        return head if room <= 0 else head + "\n" + s[-room:]
    return s[-n:]


# Stamped into every result and checked by --resume, exactly as in
# cbmc_driver.py: a resumed run reuses records verbatim, which is right
# until the meaning of a record changes and silently wrong afterwards.
#
# Bump whenever what a record MEANS changes - a check added or removed, a
# change to how a status is decided, a change to a field's source. Not
# for a bug fix that leaves the fields meaning what they meant.
#
#   1  first version. Never run against a real ESBMC.
RESULT_VERSION = 1


def _rec(task: dict, status: str, **kw) -> dict:
    return {
        "v": RESULT_VERSION,
        # Which instrument said so. cbmc_driver.py's records have no such
        # field because there was only ever one; the moment two drivers
        # write the same schema, a merged file that cannot tell them
        # apart reports the union as if one tool had said it. report.py
        # takes several .jsonl on one command line, so that file exists
        # the first time anyone runs both.
        "engine": "esbmc",
        "mode": task["mode"],
        "file": task["file"],
        "function": task["function"],
        "tier": task["tier"],
        "class": task.get("class"),
        "linkage": task.get("linkage"),
        "null_depth": 0,   # ESBMC has no known --min-null-tree-depth
        "unwind": task.get("unwind", 0),
        "max_k": task.get("max_k", 0),
        "solver": task.get("solver", "default"),
        "status": status,
        **kw,
    }


def _src_for(rel: str, preprocessed: Path | None) -> tuple[str | None, str]:
    """The file ESBMC should actually parse, and why.

    ESBMC cannot read CBMC's goto binaries, so classify.py's .gb tree is
    of no use here beyond its classes. Preference order:

      1. --preprocessed DIR/<rel>.i  - self-contained, built where the
         FreeBSD headers are. The only path that works for most of this
         tree, and the same remedy cbmc_driver.py's docstring names.
      2. hbsd/src/<rel> - honest, and on a Linux host it will mostly
         ERROR on the glibc header collision. That ERROR is a
         measurement of reach, not something to hide.
    """
    if preprocessed:
        for cand in (preprocessed / (rel + ".i"),
                     preprocessed / Path(rel).with_suffix(".i")):
            if cand.is_file():
                return str(cand), "preprocessed"
    p = SRC / rel
    if p.is_file():
        return str(p), "source"
    cpp = p.with_suffix(".cpp")
    if cpp.is_file():
        return str(cpp), "source"
    return None, "missing"


def _cflags_for(rel: str, arch: str, preprocessed: bool) -> list[str]:
    """includes.py's -I set, or nothing for an already-preprocessed unit.

    Imported lazily and defensively. includes.py is 238KB and pulls in
    the whole port ledger machinery; --selftest and the mock tests have
    no business paying for it, and a checkout where it is absent should
    still be able to run this driver against a preprocessed tree.
    """
    if preprocessed:
        return []
    try:
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from includes import include_flags, lang_flags  # noqa: E402
    except Exception:
        return []
    p = SRC / rel
    try:
        return [*lang_flags(p, rel, as_c=True),
                *include_flags(p, arch, cc="clang")]
    except Exception:
        return []


def load_tasks(plan: Path, scopes: list[str], classes: Path,
               allow: set[str], opts: dict) -> list[dict]:
    """One task per (translation unit, function the ledger says it defines).

    The same two intersections cbmc_driver.py makes, and for the same
    reasons: the ledger says which functions a FILE defines, so a
    `static inline' dragged into three hundred units is checked once
    rather than three hundred times; and `allow' keeps only the classes
    where an unguarded modular check is sound.

    The difference is what comes out of classes.json. cbmc_driver.py
    takes c["gb"], the goto binary. This takes only c["functions"] and
    c["linkage"] - the classification - because ESBMC cannot read that
    binary. The source is resolved separately by _src_for().
    """
    d = json.loads(plan.read_text())
    cls = json.loads(classes.read_text()) if classes.is_file() else {}
    pre = Path(opts["preprocessed"]) if opts.get("preprocessed") else None

    tasks: list[dict] = []
    skipped = {"no-model": 0, "class": 0, "not-in-model": 0, "no-source": 0}
    for rec in d["records"]:
        path = rec.get("path") or ""
        if scopes and not any(path.startswith(s) for s in scopes):
            continue
        if not rec.get("functions"):
            continue
        c = cls.get(path)
        if not c or not c.get("ok"):
            skipped["no-model"] += len(rec["functions"])
            continue
        src, how = _src_for(path, pre)
        if src is None:
            skipped["no-source"] += len(rec["functions"])
            continue
        fns = c["functions"]
        cflags = _cflags_for(path, opts["arch"], how == "preprocessed")
        for fn in rec["functions"]:
            if fn not in fns:
                skipped["not-in-model"] += 1
                continue
            if fns[fn] not in allow:
                skipped["class"] += 1
                continue
            t = {
                "file": path, "src": src, "src_kind": how, "function": fn,
                "class": fns[fn],
                "linkage": c.get("linkage", {}).get(fn, "?"),
                "cflags": cflags,
            }
            t.update(opts["run"])
            tasks.append(t)
    print("  skipped: " + "  ".join(f"{k}={v}" for k, v in skipped.items()),
          flush=True)
    return tasks


# ---------------------------------------------------------------------------
# --selftest
# ---------------------------------------------------------------------------

SMOKE_C = """
int pbsd_smoke_scalar(int a, int b)
{
\tint i, s = 0;
\tfor (i = 0; i < 4; i++)
\t\ts += a;
\treturn s + b;
}
"""


def selftest(args) -> int:
    """Say what is and is not known about this machine's ESBMC.

    This is the first thing to run on a machine that HAS the binary,
    because the driver was written against documentation. It reports
    three things:

      1. whether esbmc exists at all - and if not, says so as the
         run-stopping fact it is, not as a skip;
      2. for every flag this driver can pass, whether `esbmc --help'
         mentions it, beside this file's own confidence in it. A flag
         ESBMC does not know is a loud failure; a flag it knows by a
         different spelling is the silent one, and this is what finds it;
      3. if the binary is there, a real run of each mode over a
         four-line function, so the output patterns meet real output.
    """
    have = esbmc_available()
    print(f"== esbmc binary\n  looked for: {esbmc_bin()!r}")
    if not have:
        print("  NOT FOUND.\n")
        print("  Every task in a run on this machine would be recorded")
        print("  NOTRUN, and the run would exit non-zero. That is the")
        print("  whole design: a scope nobody checked must not be able to")
        print("  look like one that came back clean.\n")
    else:
        print("  found.\n")

    print("== flags this driver can pass, and how sure it is of each")
    print("   (ESBMC was never run when this was written; the confidence")
    print("    column is documentation, not evidence)")
    helptext = ""
    if have:
        try:
            h = subprocess.run([esbmc_bin(), "--help"], capture_output=True,
                               text=True, timeout=60)
            helptext = (h.stdout or "") + (h.stderr or "")
        except (OSError, subprocess.SubprocessError) as e:
            print(f"   could not run --help: {e}")
    unknown = []
    for flag, (conf, why) in sorted(FLAG_CONFIDENCE.items()):
        seen = ""
        if helptext:
            seen = "in --help" if flag in helptext else "NOT IN --help"
            if flag not in helptext:
                unknown.append(flag)
        print(f"  {flag:28s} {conf:6s} {seen}")
        if conf in ("low", "medium"):
            print(f"      {why}")

    print("\n== checks CBMC makes that this driver does NOT")
    for flag, (tier, why) in sorted(MISSING_CHECKS.items()):
        print(f"  {flag:28s} ({tier} tier)\n      {why}")

    print("\n== checks ESBMC makes by DEFAULT, which is why no --no-* "
          "flag is ever passed")
    print("  " + ", ".join(DEFAULT_ON_CHECKS))
    try:
        check_flags([esbmc_bin(), "x.c", "--no-bounds-check"])
        print("  BUG: check_flags() accepted --no-bounds-check")
        return 1
    except ValueError:
        print("  check_flags() refuses a command line carrying one: ok")

    if unknown:
        print(f"\n  {len(unknown)} flag(s) are not in this ESBMC's --help:")
        for f in unknown:
            print(f"    {f}")
        print("  Fix the spelling before trusting any verdict that used")
        print("  one. A flag that is merely absent errors out loudly; a")
        print("  flag that means something else does not.")

    if not have:
        # A selftest that cannot test anything is not a pass.
        print("\nNOTRUN  esbmc is absent, so nothing above the flag table")
        print("        was executed. This is not a clean result.")
        return 2

    print("\n== a real run of each mode on a four-line function")
    import tempfile
    rc = 0
    with tempfile.TemporaryDirectory() as td:
        c = Path(td) / "smoke.c"
        c.write_text(SMOKE_C)
        for mode in ("bounded", "kinduction", "incremental", "falsification"):
            task = {
                "file": "smoke.c", "src": str(c), "function":
                    "pbsd_smoke_scalar", "tier": "ub", "mode": mode,
                "unwind": 8, "max_k": 8, "context_bound": 2,
                "timeout": args.timeout, "solver": args.solver,
                "unwind_assert": args.unwind_assert, "mem_mb": 0,
            }
            r = verify_one(task)
            print(f"  {mode:14s} {r['status']:18s} "
                  f"{r.get('elapsed', 0):.2f}s "
                  f"{(r.get('detail') or '').splitlines()[:1]}")
            # NOFUNC here means ESBMC could not find a function that
            # is demonstrably in the file it was just handed, which
            # is the front end failing rather than a result.
            if r["status"] in ("ERROR", "NOTRUN", "NOFUNC"):
                rc = 1
    return rc


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"))
    ap.add_argument("--scope", action="append", default=[],
                    help="restrict to paths starting with this (repeatable)")
    ap.add_argument("--mode",
                    choices=["bounded", "kinduction", "incremental",
                             "falsification", "concurrency"],
                    default="bounded",
                    help="bounded is the A/B against cbmc_driver.py at the "
                         "same --unwind. kinduction is the only mode that "
                         "can return PROVED-UNBOUNDED by the inductive "
                         "step; incremental can by the forward condition.")
    ap.add_argument("--unwind", type=int, default=16,
                    help="loop bound for bounded and concurrency modes. "
                         "cbmc_driver.py's default is 16; keep them equal "
                         "or the A/B compares two different questions.")
    ap.add_argument("--max-k", type=int, default=32,
                    help="ESBMC --max-k-step, the ceiling on k for the "
                         "k-induction family")
    ap.add_argument("--k-step", type=int, default=0,
                    help="ESBMC --k-step. 0 does not pass the flag, so "
                         "ESBMC's own default applies.")
    ap.add_argument("--context-bound", type=int, default=2,
                    help="ESBMC --context-bound for --mode concurrency")
    ap.add_argument("--concurrency-checks", action="store_true",
                    help="add --data-races-check and --deadlock-check. Off "
                         "by default: they are a different question from "
                         "the UB tier and their results are not comparable "
                         "to a CBMC run.")
    ap.add_argument("--unwind-assert", choices=list(UNWIND_ASSERT_POLICY),
                    default="auto",
                    help="auto (default) passes neither flag and refuses "
                         "to say PROVED without evidence that unwinding "
                         "assertions were active; on passes "
                         "--unwinding-assertions; off passes the negative "
                         "and caps every verdict at BOUNDED")
    ap.add_argument("--solver", choices=sorted(SOLVER_FLAG), default="default")
    ap.add_argument("--timeout", type=int, default=60, help="seconds per function")
    ap.add_argument("--esbmc-timeout", type=int, default=0,
                    help="also pass ESBMC's own --timeout, as Ns. 0 does "
                         "not pass it; subprocess.run's timeout is the "
                         "authority and the argument format of ESBMC's is "
                         "one of this driver's uncertainties.")
    ap.add_argument("--memlimit", default="",
                    help="ESBMC --memlimit, e.g. 2g. --mem-mb is the "
                         "authority; this is belt and braces.")
    ap.add_argument("--mem-mb", type=int, default=0,
                    help="RLIMIT_AS per ESBMC instance, MB. 0 is uncapped, "
                         "which is right on a workstation and wrong on a "
                         "16GB runner - see _mem_capped().")
    ap.add_argument("--tier", choices=["ub", "advisory"], default="ub")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--classes", default="verify_classes.json",
                    help="output of classify.py. Its CLASSES are used; its "
                         "goto binaries cannot be, because ESBMC does not "
                         "read CBMC's format.")
    ap.add_argument("--preprocessed", default="",
                    help="a tree of self-contained .i built where the "
                         "FreeBSD headers are. Without it, most of this "
                         "tree cannot be parsed on a Linux host and comes "
                         "back ERROR - honestly.")
    ap.add_argument("--arch", default="amd64")
    ap.add_argument("--allow", default="SCALAR,VOID",
                    help="classes to check (SCALAR,VOID are sound "
                         "unguarded). ESBMC has no known equivalent of "
                         "CBMC's --min-null-tree-depth, so there is no "
                         "PROVED-ASSUMING path here and POINTER is not run.")
    ap.add_argument("--out", default="verify_esbmc_results.jsonl")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--resume", action="store_true",
                    help="skip (file, function) pairs already in --out")
    ap.add_argument("--selftest", action="store_true",
                    help="report what is known about this machine's ESBMC "
                         "and probe --help for every flag. Run this first.")
    args = ap.parse_args()

    if args.selftest:
        return selftest(args)

    # Said once, up front, on every run. The alternative is a reader who
    # finds out from the commit log.
    if not esbmc_available():
        print(f"WARNING: esbmc not found ({esbmc_bin()!r}).", flush=True)
        print("         Every task will be recorded NOTRUN and this run",
              flush=True)
        print("         will exit non-zero. Nothing will be checked.",
              flush=True)

    run_opts = {
        "mode": args.mode, "tier": args.tier, "unwind": args.unwind,
        "max_k": args.max_k, "k_step": args.k_step,
        "context_bound": args.context_bound,
        "concurrency_checks": args.concurrency_checks,
        "unwind_assert": args.unwind_assert, "solver": args.solver,
        "timeout": args.timeout, "esbmc_timeout": args.esbmc_timeout,
        "memlimit": args.memlimit, "mem_mb": args.mem_mb,
    }
    tasks = load_tasks(Path(args.plan), args.scope, Path(args.classes),
                       set(args.allow.split(",")),
                       {"run": run_opts, "arch": args.arch,
                        "preprocessed": args.preprocessed})

    out = Path(args.out)
    done: set[tuple[str, str]] = set()
    if not args.resume and out.is_file():
        # Results are APPENDED, which is what makes --resume work and is
        # silently wrong without it. Same reasoning as cbmc_driver.py:
        # two generations of answers in one file get counted together,
        # and a file that once held some other tool's .jsonl kills the
        # report on a record with no "status". `resume' means continue;
        # without it, this starts.
        out.write_text("")
    if args.resume and out.is_file():
        stale, keep = 0, []
        for line in out.read_text().splitlines():
            if not line.strip():
                continue
            try:
                r = json.loads(line)
            except ValueError:
                continue
            # A record from a DIFFERENT MODE is not an answer to this
            # run's question - a BOUNDED from --mode falsification says
            # nothing about what --mode kinduction would conclude - so it
            # is kept in the file and does not satisfy the task.
            if r.get("v") != RESULT_VERSION or r.get("engine") != "esbmc":
                stale += 1
                continue
            keep.append(line)
            if r.get("mode") != args.mode:
                continue
            # A NOTRUN is the absence of a check. Resuming over one would
            # make a missing binary permanent: the second run, on a
            # machine that HAS esbmc, would skip every function the first
            # one failed to check. That is the exact failure this status
            # exists to prevent.
            if r.get("status") == "NOTRUN":
                continue
            try:
                done.add((r["file"], r["function"]))
            except KeyError:
                pass
        if stale:
            print(f"  {stale} result(s) were written by an older driver or "
                  f"another engine (want v={RESULT_VERSION}, "
                  f"engine=esbmc); rechecking those", flush=True)
            out.write_text("".join(l + "\n" for l in keep))
        tasks = [t for t in tasks if (t["file"], t["function"]) not in done]

    if args.limit:
        tasks = tasks[:args.limit]

    print(f"{len(tasks)} (file, function) pairs to check in --mode "
          f"{args.mode}" + (f", {len(done)} already done" if done else ""),
          flush=True)
    if not tasks:
        return 0

    counts: dict[str, int] = {}
    t0 = time.time()
    with out.open("a") as fh, ProcessPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(verify_one, t): t for t in tasks}
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            counts[r["status"]] = counts.get(r["status"], 0) + 1
            fh.write(json.dumps(r) + "\n")
            fh.flush()
            if r["status"] == "FAILED":
                print(f"  FAILED {r['file']}:{r['function']}", flush=True)
                for f in r.get("failures", [])[:3]:
                    print(f"      {f['desc'][:150]}", flush=True)
            if i % 200 == 0 or i == len(tasks):
                rate = i / max(1e-9, time.time() - t0)
                print(f"  [{i}/{len(tasks)}] {rate:.1f}/s  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)

    print("\n== totals")
    for k, v in sorted(counts.items()):
        print(f"  {k:17s} {v}")
    if counts.get("PROVED-UNBOUNDED"):
        print("\n  PROVED-UNBOUNDED means the inductive step or the forward")
        print("  condition closed the property: it holds for EVERY loop")
        print("  iteration count, with no bound in the claim. It is")
        print("  strictly stronger than PROVED and is never merged with it.")
    if counts.get("PROVED"):
        print("\n  PROVED is bounded, exactly as in cbmc_driver.py: the")
        print("  loops closed inside --unwind. BOUNDED and UNKNOWN are")
        print("  weaker and are never folded into it.")

    # A missing binary is not a clean run, and this is where that is
    # enforced rather than hoped for.
    if counts.get("NOTRUN"):
        print(f"\nFAIL  {counts['NOTRUN']} function(s) recorded NOTRUN:")
        print("      esbmc is not on PATH, so they were never checked.")
        print("      An unchecked scope and a clean one are the same")
        print("      number in a total, which is why this cannot exit 0.")
        return 1

    # Same gate as cbmc_driver.py, and the floor is ALL of them: an ERROR
    # is a normal result for a unit ESBMC cannot parse, so 1257 of 1258
    # is a bad day. Nothing at all is a broken pipeline - most often the
    # --preprocessed tree is not where this process can see it, since
    # ESBMC cannot fall back on classify.py's goto binaries.
    checked = sum(v for k, v in counts.items() if k != "ERROR")
    if tasks and checked == 0:
        print(f"\nFAIL  {len(tasks)} function(s) and not one could be")
        print("      checked. Every result is ERROR, so this run measured")
        print("      the instrument rather than the tree - most often the")
        print("      sources are unparseable on this host and no")
        print("      --preprocessed tree was given.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
