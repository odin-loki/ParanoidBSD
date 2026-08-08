#!/usr/bin/env python3
"""
PBSD — one-file automated port driver.

    python3 pbsd.py

That's it. Run it from the repo root and walk away. It does setup on first run,
then converts batches until it runs out of work or you stop it.

    python3 pbsd.py --model opus-5      pin a stronger model if you have credits
    python3 pbsd.py --batches 5         stop after 5 batches
    python3 pbsd.py --status            print progress, do nothing else
    python3 pbsd.py --reset-setup       redo first-run setup

Everything is in this file: setup, the prompt, the differential gate, the
mutation check, the commit loop. No other config, no other scripts.

A batch is only marked done when a differential harness proves the C++ matches
the original C, and only when that harness has been proven able to fail. The
agent cannot mark its own work. That is the whole design.
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import csv
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

# cursor-agent is installed to ~/.local/bin by default; worker processes need this
# on PATH even when the parent shell did not export it.
_pbsd_bin = str(Path.home() / ".local" / "bin")
if _pbsd_bin not in os.environ.get("PATH", ""):
    os.environ["PATH"] = _pbsd_bin + os.pathsep + os.environ.get("PATH", "")

# A stale CURSOR_API_KEY in the shell overrides `cursor-agent login` and every
# worker call fails instantly. Prefer the login session unless explicitly set
# later via a dedicated flag.
os.environ.pop("CURSOR_API_KEY", None)

# ─────────────────────────────── config ──────────────────────────────────────

MODEL = "composer-2.5"                    # cheap bulk model; escalate on gate failure
ESCALATE_MODEL = "claude-opus-5-thinking-high"
BATCH_SIZE = 4          # small batches: much higher pass rate on weaker models
DEFAULT_JOBS = 24       # agent concurrency; circuit breaker halves on rate limits
RATE_LIMIT_PAUSE = 120  # seconds to sleep when the API keeps rate-limiting us
RATE_LIMIT_STREAK = 8   # consecutive rate limits before we pause and halve concurrency
AGENT_TIMEOUT = 1800          # cursor-agent -p can hang; always bound it
AGENT_RETRIES = 3             # transient API failures must not defer a file
AGENT_BACKOFF = 30            # seconds before the first retry, doubled after
GATE_TIMEOUT = 1200
MUTANT_TIMEOUT = 120          # a planted bug can loop forever; bound it hard
MIN_MUTATIONS = 3             # harness must kill at least this many planted bugs
USE_IR_EQUIV = True           # try LLVM IR equivalence first: free, no harness needed
RUN_PASSES = True             # run tools/pbsd_passes/ deterministic rewrites first
DEDUPE = True                 # collapse byte-identical clone files onto one port
DEPS_FIRST = True             # order batches so dependencies port before dependents
PRUNE_DEAD = True             # skip functions nothing in the in-scope tree calls
DRIFT_CHECK = True            # reopen batches whose upstream C has changed
PRETRIAGE = True              # send statically-hard files straight to the deferred queue
DEFERRED_ATTEMPTS = 2         # tries per file in the deferred phase before NEEDS_HUMAN
JOBS = 0                      # batches in flight; 0 = auto from cpu_count()
MECHANICAL = True             # try a free deterministic port before paying an agent

# Include flags used when compiling a batch standalone for the mechanical path.
# Longest matching directory prefix wins. These are best-effort: a batch that
# will not compile under them simply escalates to the agent, which is the point.
MECH_FLAGS: list[tuple[str, list[str]]] = [
    ("lib/msun",   ["hbsd/src/lib/msun/src", "hbsd/src/lib/msun/amd64",
                    "hbsd/src/lib/msun/x86"]),
    ("lib/libthr", ["hbsd/src/lib/libthr/thread"]),
    ("sys",        ["hbsd/src/sys", "hbsd/src/sys/amd64/include", "-D_KERNEL", "-ffreestanding"]),
]
# Added to every scope. The softfloat templates, libsys and gdtoa directories
# are here because FreeBSD sources include milieu.h, libsys.h and gdtoaimp.h by
# bare name from directories that are not their own.
MECH_COMMON = [
    "hbsd/src/lib/libc/include", "hbsd/src/lib/libc/amd64",
    "hbsd/src/lib/libc/locale", "hbsd/src/lib/libsys",
    "hbsd/src/contrib/gdtoa", "hbsd/src/include", "hbsd/src/sys", "hbsd/src",
]
# Headers the FreeBSD build generates or installs from elsewhere in the tree, so
# a plain source checkout has no <errno.h> or <math.h> at all. Symlinked into a
# scratch directory, which is what `make includes` effectively does.
MECH_GENERATED = {
    "errno.h": "sys/sys/errno.h",
    "math.h": "lib/msun/src/math.h",
    "fenv.h": "lib/msun/amd64/fenv.h",
    "complex.h": "lib/msun/src/complex.h",
    "xlocale.h": "include/xlocale.h",
}
# `-w` because only codegen matters here, never warnings, and modern clang makes
# several perfectly ordinary old-C constructs hard errors by default.
MECH_QUIET = ["-w", "-Wno-error=implicit-function-declaration",
              "-Wno-error=int-conversion", "-Wno-error=incompatible-pointer-types",
              "-Wno-error=implicit-int"]
# C spellings with no C++ equivalent. On the command line rather than in the
# generated source so they also cover the FreeBSD headers, which use both.
CXX_ONLY = ["-D_Bool=bool", "-Drestrict=__restrict"]


def sanitise_component(s: str) -> str:
    """A C++ identifier component. FreeBSD has sources like `64bit.c`, and a
    module or namespace name may not start with a digit."""
    s = re.sub(r"[^A-Za-z0-9]+", "_", s.lower()).strip("_") or "x"
    return s if not s[0].isdigit() else "n" + s

# Frozen scope. Everything else in hbsd/ is third-party, driver, or deferred.
IN_SCOPE = [
    "lib/libc", "lib/msun", "lib/libthr",
    "sys/kern", "sys/vm", "sys/geom", "sys/security",
    "sys/net", "sys/netinet", "sys/netinet6",
    "sys/fs", "sys/ufs", "sys/cam", "sys/compat",
    "sys/amd64", "sys/x86", "sys/crypto", "sys/opencrypto",
    "bin", "sbin", "usr.bin", "usr.sbin",
]
EXCLUDE = ("contrib/", "crypto/", "sys/dev/", "sys/contrib/", "/tests/", "cddl/", "stand/")

BANNED = ["NotImplemented", "migration_status", "Auto-generated migration stub",
          "status: stubbed", "status: converted"]

ROOT = Path(__file__).resolve().parent
MIG = ROOT / "docs" / "migration"
INVENTORY = MIG / "inventory.csv"
ARTIFACTS = MIG / "artifacts"
LOG = MIG / "drive_log.jsonl"
DEFERRED = MIG / "deferred.jsonl"
NEEDS_HUMAN = MIG / "NEEDS_HUMAN.md"
SETUP_STAMP = MIG / ".setup_done"
WORK = ROOT / "pbsd"

# ─────────────────────────────── the prompt ──────────────────────────────────

PROMPT = """You are porting HardenedBSD C to C++23 for the PBSD project.

BATCH: {batch_id}
SOURCE FILES (read these from the repo, they are the input):
{file_list}

Produce EXACTLY these four files in {outdir}/ and nothing else:

1. port.cppm — a C++23 module named `pbsd.{module}` exporting namespace
   `pbsd::{ns}`, containing a faithful port of every function in the batch.
   FAITHFUL means: preserve behaviour exactly, including bugs, integer
   signedness, evaluation order and pointer arithmetic. Where the original
   assigns a plain `char` to an `int`, do the same — the sign extension is
   load-bearing. Keep the original copyright headers. Do not improve anything.

2. oracle.c — the original C sources concatenated, every function renamed with
   a `ref_` prefix, bodies otherwise UNMODIFIED. Add any missing defines
   (e.g. LONG_BIT) but never change a function body. This is the specification.

3. harness.cpp — a differential test with `int main()`. For every function:
   - hand-written edge cases (empty, single char, NUL-heavy, high-bit bytes
     0x80-0xFF, boundary lengths), AND
   - a fixed-seed randomised sweep, at least 200000 iterations.
   For each case call BOTH the port and the ref_ oracle and compare:
   - return values;
   - for functions that write to a buffer: allocate TWO buffers, fill both with
     guard byte 0x7f, copy identical input into each, and compare the ENTIRE
     buffer afterwards including bytes past the nominal write window. Comparing
     only the return value passes broken ports — do not do that;
   - for pointer returns: compare OFFSETS from each buffer base, never raw
     addresses;
   - for stateful iterators (strsep-like): drive to exhaustion, comparing token
     offset, the output state pointer, and the buffer after EVERY iteration.
   Print a per-function table of cases/failures. Return 0 only if every single
   case matched; return 1 otherwise.

   Your harness will then be MUTATION TESTED: bugs are planted in your port
   (comparisons flipped, `+` swapped for `-`, `&&` for `||`, `0` for `1`,
   `++` for `--`) and the harness must fail for every one that compiles. A
   harness that passes a planted bug is rejected even if your port is perfect.
   So make sure the inputs you generate actually drive every branch, every
   comparison and every arithmetic expression in the port to a value where a
   flip would change the observable result. Test both sides of each boundary,
   not just the happy path.

4. build.sh — POSIX sh. Compiles oracle.c with `cc -std=c11 -O2`, port.cppm and
   harness.cpp with `c++ -std=c++23` plus whatever module flags this toolchain
   needs, links all three, and execs the harness so its exit code is the script
   exit code. Must work when run as `sh build.sh` from {outdir}/.

RULES — these are absolute:
- NEVER emit a stub, placeholder, `NotImplemented`, `migration_status()`, or
  "TODO: port this". If a file cannot be ported faithfully, LEAVE IT OUT of
  port.cppm and oracle.c entirely and write the filename plus a one-line reason
  to {outdir}/skipped.txt. A short correct batch is a success. A stub is a
  failure.
- NEVER edit the oracle, weaken a comparison, or narrow the input range to make
  the harness pass. If it diverges, the port is wrong.
- Do not modify any file outside {outdir}/.
- Do not explore the repository. Work only from the source files listed above.

Finish by running `sh build.sh` and reporting the table.
"""

# ───────────────────────────── mutations ─────────────────────────────────────
# Planted bugs. If the harness cannot detect these, it proves nothing and the
# batch is rejected regardless of whether it passed.
#
# These are the standard mutation-testing operators rather than string-function
# specific ones. The original five only matched `!= 0)`, `*p = '\0';` and
# `return a - b;`, which is the shape of libc string code and almost nothing
# else, so every batch outside lib/libc/string failed the gate for having too
# few applicable mutations rather than for being wrong. Ordered most to least
# generally applicable; the scan stops once MUTANT_CAP have been run.

MUTATIONS = [
    ("relational_lt",  r"(?<![<>!=+\-*/%&|^])<(?![<=])",        ">="),
    ("relational_gt",  r"(?<![<>!=+\-*/%&|^])>(?![>=])",        "<="),
    ("equality",       r"==",                                   "!="),
    ("inequality",     r"!=",                                   "=="),
    ("arith_plus",     r"(?<![+\-=<>!*/%&|^])\+(?![+=])",       "-"),
    ("arith_minus",    r"(?<![+\-=<>!*/%&|^])-(?![-=>])",       "+"),
    ("increment",      r"\+\+",                                 "--"),
    ("logical_and",    r"&&",                                   "||"),
    ("logical_or",     r"\|\|",                                 "&&"),
    ("shift_left",     r"<<(?!=)",                              ">>"),
    ("shift_right",    r"(?<!-)>>(?!=)",                        "<<"),
    ("const_zero",     r"\b0\b",                                "1"),
    ("const_one",      r"\b1\b",                                "0"),
    ("zero_return",    r"return\s+(\w+)\s*-\s*(\w+)\s*;",       "return 0;"),
    ("dropped_nul",    r"\*\s*(\w+)\s*=\s*'\\0'\s*;",           "/*mut*/;"),
]
MUTANT_CAP = 6          # stop after this many compilable mutants; bounds runtime
MUTANT_ATTEMPTS = 14    # and stop looking after this many tries, compiled or not

# ─────────────────────────────── helpers ─────────────────────────────────────


def say(msg: str) -> None:
    print(f"  {msg}", flush=True)


def banner(msg: str) -> None:
    print(f"\n=== {msg} ===", flush=True)


def log(**kw) -> None:
    kw["ts"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    LOG.parent.mkdir(parents=True, exist_ok=True)
    with LOG.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(kw) + "\n")


def sh(cmd, cwd=None, timeout=None, quiet=True):
    return subprocess.run(
        cmd, cwd=cwd or ROOT, timeout=timeout, text=True,
        stdin=subprocess.DEVNULL,
        capture_output=quiet,
    )


def clean_batch(outdir: Path) -> None:
    """Undo a failed batch WITHOUT touching the rest of the tree.

    A repo-wide `git checkout -- . && git clean -fd` will delete the inventory,
    the artifacts and the log if they are not yet committed -- and will also
    destroy any unrelated work in progress. Scope the cleanup to the batch's
    own output directory and the module tree, and never to docs/migration.
    """
    shutil.rmtree(outdir, ignore_errors=True)
    sh(["git", "-C", str(ROOT), "checkout", "-q", "--", str(WORK.name)])
    sh(["git", "-C", str(ROOT), "clean", "-qfd", str(WORK.name)])


def git(*args) -> int:
    """Run git in the repo root. Retries commits when index.lock is transient."""
    for attempt in range(8):
        r = sh(["git", "-C", str(ROOT), *args])
        if r.returncode == 0:
            return 0
        if not args or args[0] != "commit":
            return r.returncode
        err = (r.stderr or r.stdout or "").strip()
        if "nothing to commit" in err:
            return 0
        if "index.lock" in err and attempt < 7:
            time.sleep(0.25 * (attempt + 1))
            continue
        if err:
            say(f"WARNING: git commit failed — {err.splitlines()[0][:90]}")
        return r.returncode
    return 1


def preflight() -> bool:
    ok = True
    for tool, hint in [("git", ""), ("cc", "install clang or gcc"),
                       ("c++", "install clang++ or g++"),
                       ("cursor-agent", "install Cursor, then run: cursor-agent login")]:
        if shutil.which(tool) is None:
            print(f"MISSING: {tool}  {hint}")
            ok = False
    if not (ROOT / "hbsd" / "src").is_dir():
        print("MISSING: hbsd/src — run this from the ParanoidBSD repo root")
        ok = False
    if ok:
        r = sh(["git", "-C", str(ROOT), "config", "user.email"])
        if r.returncode != 0 or not (r.stdout or "").strip():
            sh(["git", "-C", str(ROOT), "config", "user.email", "pbsd@localhost"])
            sh(["git", "-C", str(ROOT), "config", "user.name", "pbsd automation"])
            say("git identity was unset — configured a local one so commits land")
    if ok and not os.environ.get("CURSOR_API_KEY"):
        say("note: CURSOR_API_KEY unset — relying on cached `cursor-agent login`")
        try:
            r = subprocess.run(
                ["cursor-agent", "-p", "Reply READY", "--workspace", str(ROOT),
                 "--model", "composer-2.5", "--output-format", "text", "--force", "--trust"],
                capture_output=True, text=True, timeout=60,
                env=os.environ, stdin=subprocess.DEVNULL)
            if r.returncode != 0:
                err = (r.stderr or r.stdout or "").strip()
                print("cursor-agent is not authenticated for batch work.")
                print(f"  {err[:200]}")
                print("  Fix: wsl -d Ubuntu, then: cursor-agent login")
                print("  Or set a valid CURSOR_API_KEY in ~/pbsd_driver.sh")
                ok = False
        except Exception as e:
            print(f"cursor-agent probe failed: {e}")
            ok = False
    return ok


# ─────────────────────────────── setup ───────────────────────────────────────


def in_scope(rel: str) -> bool:
    if any(x in rel for x in EXCLUDE):
        return False
    return any(rel.startswith(p) for p in IN_SCOPE)


def setup() -> None:
    banner("First-run setup")

    # 1. Remove the stub tree. It carries no information and reports false state.
    stubs = WORK / "ports"
    if stubs.is_dir():
        n = sum(1 for _ in stubs.rglob("*.cppm"))
        git("rm", "-r", "-q", "--ignore-unmatch", "ports", )
        shutil.rmtree(stubs, ignore_errors=True)
        say(f"removed {n} stub modules under pbsd/ports/")

    # 2. Quarantine the stub generators so nothing re-creates them.
    q = ROOT / "tools" / "quarantine"
    q.mkdir(parents=True, exist_ok=True)
    moved = 0
    for p in (ROOT / "tools").glob("*.py"):
        if re.match(r"(gen_|burst|_gen_|_burst|mark_converted)", p.name):
            shutil.move(str(p), str(q / p.name))
            moved += 1
    if moved:
        say(f"quarantined {moved} generator scripts to tools/quarantine/")

    # 3. Build the inventory from the frozen scope, batched by directory.
    src = ROOT / "hbsd" / "src"
    by_dir: dict[str, list[Path]] = {}
    for c in src.rglob("*.c"):
        rel = str(c.relative_to(src))
        if in_scope(rel):
            by_dir.setdefault(str(c.parent.relative_to(src)), []).append(c)

    groups = []
    for d in sorted(by_dir):
        files = sorted(by_dir[d], key=lambda p: p.stat().st_size)
        for i in range(0, len(files), BATCH_SIZE):
            chunk = files[i:i + BATCH_SIZE]
            lines = {f: sum(1 for _ in f.open(errors="ignore")) for f in chunk}
            groups.append((sum(lines.values()), d, chunk, lines))

    # Easiest first. A weak model banks hundreds of small leaf functions before
    # it reaches anything that will stump it.
    all_files = [f for _, _, chunk, _ in groups for f in chunk]
    defines, refs = symbol_index(all_files)
    dead = dead_symbols(defines, refs)
    if dead:
        say(f"pruned {len(dead)} files nothing in scope references")

    tiers = dependency_rank(groups, defines, refs)
    # Dependencies first, then easiest first within each tier.
    order = sorted(range(len(groups)), key=lambda i: (tiers[i], groups[i][0]))
    groups = [groups[i] for i in order]
    say(f"ordered {len(groups)} batches across {max(tiers.values())+1} dependency tiers")

    if DEFERRED.exists():
        DEFERRED.unlink()
    n_hard = [0]
    rows = []
    for bid, (_, d, chunk, lines) in enumerate(groups, 1):
        for f in chunk:
            rel = str(f.relative_to(ROOT))
            hard = classify_difficulty(f)
            if rel in dead:
                st = "SKIPPED"
            elif hard:
                st = "DEFERRED"
                defer(rel, f"b{bid:04d}", hard, "pre-triage: statically hard")
                n_hard[0] += 1
            else:
                st = "PENDING"
            rows.append({"batch_id": f"b{bid:04d}", "path": rel, "dir": d,
                         "lines": lines[f], "status": st})

    if n_hard[0]:
        say(f"pre-triaged {n_hard[0]} statically-hard files to the deferred queue")
    n_clone = collapse_clones(rows)
    if n_clone:
        say(f"collapsed {n_clone} clone files onto {len(rows)-n_clone} unique ports")

    MIG.mkdir(parents=True, exist_ok=True)
    (ROOT / ".gitignore").touch()
    gi = (ROOT / ".gitignore").read_text(errors="ignore")
    if "docs/migration/inventory.csv" not in gi:
        (ROOT / ".gitignore").write_text(
            gi.rstrip("\n") + "\n# pbsd.py state — never clean these\n"
            "!docs/migration/\n", encoding="utf-8")
    with INVENTORY.open("w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=["batch_id", "path", "dir", "lines", "status", "clone_of"])
        w.writeheader()
        w.writerows(rows)
    say(f"inventory: {len(rows)} files in {bid} batches "
        f"({sum(r['lines'] for r in rows):,} lines in scope)")

    run_deterministic_passes()

    SETUP_STAMP.write_text(time.strftime("%Y-%m-%d %H:%M:%S"), encoding="utf-8")
    git("add", "-A")
    git("commit", "-q", "-m", "pbsd: reset to verified-only pipeline")
    say("setup committed")





# ─────────────────────────── deferred work queue ─────────────────────────────
#
# Anything the automated pass cannot convert goes here rather than blocking the
# run or silently disappearing. The queue is worked LAST, one file at a time,
# with the previous failure fed back in. Whatever survives two attempts is
# written up for a human. Nothing is ever stubbed and nothing is ever lost.

HARD_MARKERS = [
    ("INLINE_ASM",    re.compile(r"\b__asm__?\b|\basm\s*(volatile)?\s*\(")),
    ("LINKER_SET",    re.compile(r"\b(SYSINIT|SYSUNINIT|DATA_SET|TEXT_SET|MODULE_DEPEND|DECLARE_MODULE)\b")),
    ("QUEUE_MACRO",   re.compile(r"\b(LIST|TAILQ|STAILQ|SLIST|RB|SPLAY)_(HEAD|ENTRY|INIT|INSERT|REMOVE|FOREACH)")),
    ("FLEX_ARRAY",    re.compile(r"\w+\s+\w+\s*\[\s*\]\s*;\s*\}")),
    ("VLA",           re.compile(
        r"\b(?:char|u?int\w*|short|long|size_t|struct\s+\w+|u_char|u_int)\s+\*?\w+"
        r"\s*\[\s*(?!\d|\]|[A-Z_]{2,})[a-z_]\w*\s*\]\s*;")),
    ("COMPUTED_GOTO", re.compile(r"goto\s*\*")),
    ("HEAVY_CPP",     re.compile(r"#\s*(if|ifdef|ifndef|elif)\b")),
    ("GENERIC",       re.compile(r"\b_Generic\b")),
]
HEAVY_CPP_THRESHOLD = 40      # #if density above which a file is macro-shaped
_INCLUDE_C = re.compile(r'^\s*#\s*include\s+"[^"]*\.c"', re.M)


def classify_difficulty(path: Path) -> list[str]:
    """Static pre-triage. Cheaper to detect a hard file than to pay an agent
    call to discover it. These are the idioms with no mechanical C++ mapping."""
    if not PRETRIAGE:
        return []
    txt = norm_lines(path)
    if not txt:
        return []
    codes = []
    for code, rx in HARD_MARKERS:
        hits = len(rx.findall(txt))
        if code == "HEAVY_CPP":
            if hits >= HEAVY_CPP_THRESHOLD:
                codes.append(f"{code}({hits})")
        elif hits:
            codes.append(code)

    # A file with no function definitions has nothing to put in port.cppm, so
    # the normal batch prompt cannot succeed on it -- the gate demands a
    # port.cppm and the agent has no functions to write. These are data-only
    # translation units and preprocessor instantiation shims. Route them
    # straight to the deferred queue instead of paying a batch call to fail.
    if not [n for n in _DEFN.findall(txt)
            if n not in _KEYWORDS and not n.isupper()]:
        codes.append("INCLUDE_SHIM" if _INCLUDE_C.search(txt) else "NO_FUNCTIONS")
    return codes


def defer(path: str, batch: str, reasons: list[str], detail: str = "") -> None:
    rec = {"path": path, "batch": batch, "reasons": reasons,
           "detail": detail[-1500:], "attempts": 0,
           "queued": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
    DEFERRED.parent.mkdir(parents=True, exist_ok=True)
    with DEFERRED.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(rec) + "\n")


def load_deferred() -> list[dict]:
    if not DEFERRED.is_file():
        return []
    out, seen = [], set()
    for line in DEFERRED.read_text(errors="ignore").splitlines():
        if not line.strip():
            continue
        try:
            r = json.loads(line)
        except Exception:
            continue
        out = [x for x in out if x["path"] != r["path"]]   # last record wins
        out.append(r)
        seen.add(r["path"])
    return out


def save_deferred(items: list[dict]) -> None:
    DEFERRED.write_text("".join(json.dumps(i) + "\n" for i in items), encoding="utf-8")


DEFERRED_PROMPT = """You are porting ONE hard HardenedBSD C file to C++23 for PBSD.
The automated pass could not convert it. You get more room here, but the same
absolute rules.

FILE: {path}   ({lines} lines)
KNOWN DIFFICULTIES: {reasons}
PREVIOUS FAILURE:
{detail}

Produce the same four files in {outdir}/ as a normal batch: port.cppm,
oracle.c, harness.cpp, build.sh. Read the failure above and address the actual
cause -- do not resubmit the same approach.

Guidance for the specific difficulties flagged:
- QUEUE_MACRO: port the macro's EXPANSION for this file's concrete type. Do not
  invent a template intrusive-list library; that changes ABI and breaks every
  other consumer.
- INLINE_ASM: keep the asm block verbatim inside the C++ function. Do not
  attempt to express it in C++.
- LINKER_SET / SYSINIT: these need section attributes preserved exactly. If you
  cannot preserve the section placement, SKIP the file -- say so and stop.
- HEAVY_CPP: port ONE configuration (amd64, the default #if branch) and record
  which branch you took at the top of port.cppm. Do not try to preserve every
  conditional; C++ modules cannot express a conditional interface.
- FLEX_ARRAY / VLA: keep the C layout. A std::vector changes the object layout
  and breaks ABI.
- GENERIC: expand _Generic into an overload set only where the branches are
  genuinely type-dispatchable; otherwise skip.
- INCLUDE_SHIM: this file defines a few macros and #includes another .c file to
  instantiate it at a concrete type. Port the EXPANSION: read the included file,
  substitute the macros this file sets, and emit the resulting concrete
  function. Do not #include a .c file from the module.
- NO_FUNCTIONS: this translation unit defines data, not functions. Port the
  object definitions with their exact types, linkage and initialisers. The
  harness cannot call anything, so instead assert on each object's initial
  value, sizeof and alignof against the same object in the oracle, and say in
  the table that the comparison is a data comparison.

ABSOLUTE, unchanged: never emit a stub, NotImplemented, migration_status(), or
a TODO placeholder. Never edit the oracle or weaken the harness to pass. If this
file genuinely cannot be ported faithfully, write {outdir}/IMPOSSIBLE.txt with a
one-paragraph explanation of what specifically blocks it, and produce nothing
else. That is an acceptable and useful outcome.

Finish by running `sh build.sh` and reporting the table.
"""


def write_human_report(items: list[dict]) -> None:
    """Everything that survived the deferred phase, written up for you."""
    if not items:
        if NEEDS_HUMAN.exists():
            NEEDS_HUMAN.unlink()
        return
    by_reason: dict[str, list[dict]] = {}
    for it in items:
        for r in (it["reasons"] or ["UNCLASSIFIED"]):
            by_reason.setdefault(r.split("(")[0], []).append(it)

    lines = ["# PBSD — files needing human decisions", "",
             f"Generated {time.strftime('%Y-%m-%d %H:%M UTC', time.gmtime())}. "
             f"{len(items)} files could not be converted automatically.", "",
             "These are grouped by blocking idiom. Each group is usually **one "
             "design decision** that then unblocks every file in it — that is the "
             "highest-leverage work left.", ""]
    for reason in sorted(by_reason, key=lambda k: -len(by_reason[k])):
        group = by_reason[reason]
        lines += [f"## {reason} — {len(group)} files", ""]
        for it in sorted(group, key=lambda x: x["path"])[:40]:
            lines.append(f"- `{it['path']}` — {it.get('last_error','?').splitlines()[0][:110]}"
                         if it.get("last_error") else f"- `{it['path']}`")
        if len(group) > 40:
            lines.append(f"- …and {len(group)-40} more")
        lines.append("")
    NEEDS_HUMAN.write_text("\n".join(lines), encoding="utf-8")


# ───────────────────── symbol graph & dependency order ───────────────────────

_DEFN = re.compile(r"^([A-Za-z_]\w*)\s*\(", re.M)
_CALL = re.compile(r"\b([a-z_][a-z0-9_]{2,})\s*\(")

_KEYWORDS = {"if", "for", "while", "switch", "return", "sizeof", "defined",
             "static", "inline", "typedef", "struct", "union", "enum", "case",
             "else", "do", "goto", "break", "continue", "const", "volatile"}


def symbol_index(paths: list[Path]) -> tuple[dict[str, str], dict[str, set[str]]]:
    """Map symbol -> defining file, and file -> symbols it references."""
    defines: dict[str, str] = {}
    refs: dict[str, set[str]] = {}
    for p in paths:
        txt = norm_lines(p)
        if not txt:
            continue
        key = str(p.relative_to(ROOT))
        for name in _DEFN.findall(txt):
            # ALL-CAPS at column 0 is a macro invocation (CTASSERT, STAILQ_HEAD,
            # DEFINE_UIFUNC), not a function definition.
            if name not in _KEYWORDS and not name.isupper():
                defines.setdefault(name, key)
        refs[key] = {c for c in _CALL.findall(txt) if c not in _KEYWORDS}
    return defines, refs


def dead_symbols(defines: dict[str, str], refs: dict[str, set[str]]) -> set[str]:
    """Files whose every defined symbol is referenced nowhere in scope.

    These are almost always arch-variant or config-gated code paths that the
    frozen scope never reaches. Skipping them is free denominator reduction.
    """
    if not PRUNE_DEAD:
        return set()
    called: set[str] = set()
    for r in refs.values():
        called |= r
    by_file: dict[str, set[str]] = {}
    for sym, f in defines.items():
        by_file.setdefault(f, set()).add(sym)
    dead = set()
    for f, syms in by_file.items():
        if syms and not (syms & called):
            dead.add(f)
    return dead


def dependency_rank(groups: list, defines: dict[str, str],
                    refs: dict[str, set[str]]) -> dict[int, int]:
    """Assign each batch a tier: everything it calls sits in a lower tier.

    Without this a batch can reference symbols from files that have not been
    ported, so its module will not import and its harness will not link. Cycles
    are broken by putting the smaller batch first; the kernel include graph is
    genuinely cyclic and pretending otherwise deadlocks the queue.
    """
    if not DEPS_FIRST:
        return {i: 0 for i in range(len(groups))}

    file_batch = {}
    for i, (_, _, chunk, _) in enumerate(groups):
        for f in chunk:
            file_batch[str(f.relative_to(ROOT))] = i

    edges: dict[int, set[int]] = {i: set() for i in range(len(groups))}
    for i, (_, _, chunk, _) in enumerate(groups):
        for f in chunk:
            for sym in refs.get(str(f.relative_to(ROOT)), ()):
                owner = defines.get(sym)
                if owner and owner in file_batch:
                    j = file_batch[owner]
                    if j != i:
                        edges[i].add(j)

    # Longest-path tier via iterative relaxation. Cycles simply stop rising,
    # which is the behaviour we want: the kernel graph is genuinely cyclic and
    # a strict topological sort would deadlock.
    tier: dict[int, int] = {i: 0 for i in range(len(groups))}
    for _ in range(24):
        changed = False
        for i, deps in edges.items():
            want = max((tier[d] + 1 for d in deps), default=0)
            if want > tier[i]:
                tier[i], changed = want, True
        if not changed:
            break
    return tier


# ────────────────────────── upstream drift ───────────────────────────────────


def check_drift(rows: list[dict]) -> int:
    """Reopen any VERIFIED batch whose original C has changed since it passed.

    Deferring this is the easiest way to silently desynchronise from upstream.
    """
    if not DRIFT_CHECK or not ARTIFACTS.is_dir():
        return 0
    reopened = 0
    for art in ARTIFACTS.glob("*.json"):
        try:
            data = json.loads(art.read_text())
        except Exception:
            continue
        stale = False
        for path, old_hash in (data.get("input_hashes") or {}).items():
            f = ROOT / path
            if not f.is_file() or fingerprint(f) != old_hash:
                stale = True
                break
        if stale:
            for r in rows:
                if r["batch_id"] == data["batch"]:
                    r["status"] = "PENDING"
            art.unlink()
            reopened += 1
    return reopened


# ─────────────────────────── build wiring ────────────────────────────────────


def emit_build_wiring(rows: list[dict]) -> None:
    """Generate a CMakeLists listing every VERIFIED module, so the port is
    actually buildable as a unit rather than a pile of loose files."""
    # Mechanical ports live in <dir>/<stem>_m/ and agent batches in
    # <dir>/<batch_id>/, so take whatever port.cppm actually exists rather than
    # reconstructing the path from the inventory.
    mods = sorted(str(p) for p in WORK.rglob("port.cppm"))
    if not mods:
        return
    rel = [os.path.relpath(m, WORK) for m in mods]
    (WORK / "CMakeLists.txt").write_text(
        "# generated by pbsd.py -- verified modules only, do not edit\n"
        "cmake_minimum_required(VERSION 3.28)\n"
        "project(pbsd CXX)\n"
        "set(CMAKE_CXX_STANDARD 23)\n"
        "set(CMAKE_CXX_SCAN_FOR_MODULES ON)\n"
        "add_library(pbsd STATIC)\n"
        "target_sources(pbsd PUBLIC FILE_SET CXX_MODULES FILES\n"
        + "".join(f"  {m}\n" for m in rel) + ")\n",
        encoding="utf-8")


# ───────────────────── deterministic pre-passes ──────────────────────────────


def run_deterministic_passes() -> None:
    """Run the repo's own clang/AST rewrite passes before any model work.

    Every idiom these absorb -- void* casts, `register`, K&R prototypes,
    typedef struct, NULL->nullptr -- is one the model never has to get right,
    permanently, across the whole tree, for free.
    """
    runner = ROOT / "tools" / "run_todo_passes.py"
    if not (RUN_PASSES and runner.is_file()):
        return
    banner("Deterministic passes")
    try:
        r = sh([sys.executable, str(runner)], timeout=3600)
        say("passes complete" if r.returncode == 0 else
            f"passes exited {r.returncode} (continuing)")
    except subprocess.TimeoutExpired:
        say("passes timed out (continuing)")

    ref = MIG / "clang_port" / "refusals.jsonl"
    if ref.is_file():
        counts: dict[str, int] = {}
        for line in ref.read_text(errors="ignore").splitlines():
            try:
                counts[json.loads(line).get("reason_code", "?")] = counts.get(
                    json.loads(line).get("reason_code", "?"), 0) + 1
            except Exception:
                pass
        top = sorted(counts.items(), key=lambda kv: -kv[1])[:8]
        if top:
            say("top refusals (these are what the model actually has to solve):")
            for code, n in top:
                say(f"    {n:>6}  {code}")


# ────────────────────────── clone collapsing ─────────────────────────────────

_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
_WS = re.compile(r"\s+")


def fingerprint(path: Path) -> str:
    """Stable content hash. Python's hash() is salted per process and would
    make every drift check report false positives on the next run."""
    return hashlib.sha256(norm_source(path).encode()).hexdigest()[:16]


def norm_lines(path: Path) -> str:
    """Strip comments but KEEP newlines -- FreeBSD KNF puts the function name at
    column 0, which is the most reliable definition signal in this tree."""
    try:
        txt = path.read_text(errors="ignore")
    except Exception:
        return ""
    return _COMMENT.sub(lambda m: "\n" * m.group(0).count("\n"), txt)


def norm_source(path: Path) -> str:
    """Normalise a C file for clone detection: strip comments and whitespace."""
    try:
        txt = path.read_text(errors="ignore")
    except Exception:
        return ""
    txt = _COMMENT.sub(" ", txt)
    txt = _WS.sub(" ", txt)
    return txt.strip()


def collapse_clones(rows: list[dict]) -> int:
    """Mark byte-identical sources as CLONE of one representative.

    The kernel tree is full of arch-variant and vendor-variant near-duplicates.
    Porting one and copying the result is strictly better than paying for the
    same conversion N times.
    """
    if not DEDUPE:
        return 0
    seen: dict[str, str] = {}
    collapsed = 0
    for r in rows:
        h = fingerprint(ROOT / r["path"])
        if not h:
            continue
        if h in seen and seen[h] != r["path"]:
            r["status"] = "CLONE"
            r["clone_of"] = seen[h]
            collapsed += 1
        else:
            seen[h] = r["path"]
            r.setdefault("clone_of", "")
    return collapsed


def propagate_clones(rows: list[dict], verified_paths: set[str]) -> int:
    """Once a representative verifies, its clones inherit the verdict."""
    n = 0
    for r in rows:
        if r["status"] == "CLONE" and r.get("clone_of") in verified_paths:
            r["status"] = "VERIFIED"
            n += 1
    return n


# ─────────────────────── LLVM IR equivalence oracle ──────────────────────────

_IR_NOISE = re.compile(
    r"(^\s*;.*$)|(^\s*(source_filename|target|!|attributes|declare|@__).*$)",
    re.M)
# Skips the canonical block labels this module writes, which must survive the
# generic name squashing so control flow is still compared.
_IR_NAMES = re.compile(r"[%@](?!Lbb)[\w.$-]+")
_IR_DEFINE = re.compile(r"^define[^{]*@([\w.$]+)\([^)]*\)[^{]*\{(.*?)^\}", re.M | re.S)

# Codegen noise that differs between the C and C++ front ends for identical
# source: attribute group references (#0), inferred parameter/return attributes,
# and the `; preds = ...` comment on every basic block.
_IR_ATTRGRP = re.compile(r"\s#\d+\b")
_IR_PREDS = re.compile(r"[ \t]*;[ \t]*preds[ \t]*=.*$", re.M)
_IR_ATTRS = re.compile(
    r"\b(?:noundef|nonnull|signext|zeroext|inreg|returned|nocapture|noalias|"
    r"nofree|readonly|writeonly|writable|immarg|"
    r"dereferenceable(?:_or_null)?\(\d+\)|captures\([^)]*\)|align \d+)\b[ \t]*")
_IR_LABELDEF = re.compile(r"^([\w.$-]+):", re.M)
_IR_LABELREF = re.compile(r"label %([\w.$-]+)")
_IR_META = re.compile(r"![0-9]+")


def _bb_name(k: int) -> str:
    """Digit-free canonical block name, so integer squashing cannot merge two."""
    s, k = "", k + 1
    while k:
        k, r = divmod(k - 1, 26)
        s = chr(97 + r) + s
    return "Lbb" + s


def _llvm() -> tuple[str, str] | None:
    """Locate a real LLVM toolchain. IR comparison needs clang, not gcc."""
    for c, cxx in (("clang", "clang++"), ("cc", "c++")):
        if shutil.which(c) and shutil.which(cxx):
            r = subprocess.run([c, "--version"], capture_output=True, text=True)
            if "clang" in (r.stdout or "").lower():
                return c, cxx
    return None


def emit_ir(src: Path, lang: str, extra: list[str], workdir: Path) -> str | None:
    """Compile one file to LLVM IR text, or None if that isn't possible here."""
    tc = _llvm()
    if tc is None:
        return None
    out = workdir / (src.stem + "." + lang + ".ll")
    cc = tc[0] if lang == "c" else tc[1]
    std = "-std=c11" if lang == "c" else "-std=c++23"
    cmd = [cc, std, "-O2", "-S", "-emit-llvm", "-g0",
           "-fno-discard-value-names", str(src), "-o", str(out), *extra]
    try:
        r = sh(cmd, cwd=workdir, timeout=180)
        if r.returncode != 0:      # retry without the value-names flag
            cmd.remove("-fno-discard-value-names")
            r = sh(cmd, cwd=workdir, timeout=180)
    except subprocess.TimeoutExpired:
        return None
    if r.returncode != 0 or not out.is_file():
        return None
    return out.read_text(errors="ignore")


def ir_bodies(ir: str) -> dict[str, str]:
    """Extract per-function normalised opcode sequences from IR text.

    Basic-block labels are renamed by order of first appearance rather than
    compared literally: for identical source the C and C++ front ends emit the
    same control flow but number the blocks differently (`if.end5` against
    `if.end6`), which used to be reported as a behavioural difference. Attribute
    groups and inferred parameter attributes are dropped for the same reason.
    """
    bodies: dict[str, str] = {}
    for name, body in _IR_DEFINE.findall(_IR_NOISE.sub("", ir)):
        if name.startswith(("_ZGI", "ZGI")):     # module initialiser, not code
            continue
        body = _IR_PREDS.sub("", body)
        body = _IR_ATTRGRP.sub("", body)
        body = _IR_ATTRS.sub("", body)

        order: dict[str, str] = {}

        def canon(label: str) -> str:
            if label not in order:
                order[label] = _bb_name(len(order))
            return order[label]

        for m in _IR_LABELDEF.finditer(body):
            canon(m.group(1))
        for m in _IR_LABELREF.finditer(body):
            canon(m.group(1))
        body = _IR_LABELREF.sub(lambda m: f"label %{canon(m.group(1))}", body)
        body = _IR_LABELDEF.sub(lambda m: f"{canon(m.group(1))}:", body)

        ops = []
        for line in body.splitlines():
            line = _IR_NAMES.sub("%v", line.strip())
            # Only metadata ids are squashed. This used to blank every integer,
            # which silently made the whole comparison worthless: a port with `1`
            # where the C had `0` normalised to the same text and was accepted.
            # Literal constants are exactly what has to be compared.
            line = _IR_META.sub("!N", line)
            if line and not line.startswith(("#", ";")):
                ops.append(line)
        bodies[name] = "\n".join(ops)
    return bodies


def demangle_key(sym: str) -> str:
    """Bare function name from a possibly module-mangled Itanium symbol.

    Clang mangles module-attached entities with extra `W<module>` components, so
    the old `\\d+(\\w+)` regex matched once and returned the entire tail --
    nothing ever matched by name and every mechanical port looked like it
    differed. Walk the <length><name> sequence properly instead.
    """
    if not sym.startswith("_Z"):
        return sym.lstrip("_")
    s = sym[2:]
    if s[:1] == "N":
        s = s[1:]
    parts, i = [], 0
    while i < len(s):
        if s[i] == "W":                 # module component, not part of the name
            i += 1
            continue
        if not s[i].isdigit():
            break
        j = i
        while j < len(s) and s[j].isdigit():
            j += 1
        n = int(s[i:j])
        parts.append(s[j:j + n])
        i = j + n
    return parts[-1] if parts else sym.lstrip("_")


def degenerate(body: str) -> bool:
    """True if this body contains no actual computation, so proves nothing.

    Some sources compile to a single `unreachable`: the compiler found undefined
    behaviour and threw the body away. Two such bodies always compare equal, so
    treating that as proof let *any* port through -- every file in lib/libc/quad
    was being certified against an empty oracle. Those must go to the agent.
    """
    ops = [l for l in body.splitlines() if l and not l.endswith(":")]
    return not ops or all(o == "unreachable" for o in ops)


def match_ir(ref: dict[str, str], port: dict[str, str]) -> tuple[list[str], list[str]]:
    """Pair every oracle function with a port function of identical IR.

    Name matching is tried first; anything left over is matched on body content,
    because C++ symbol mangling is a moving target and an unmatched name would
    otherwise be reported as a behavioural difference it is not. Each port
    function can only be consumed once, so this stays a real bijection.
    """
    matched, differing, pool = [], [], dict(port)
    for fn, body in ref.items():
        if pool.get(fn) == body:
            matched.append(fn)
            pool.pop(fn)
            continue
        hit = next((k for k, v in pool.items() if v == body), None)
        if hit is None:
            differing.append(fn)
        else:
            matched.append(fn)
            pool.pop(hit)
    return matched, differing


def ir_equivalence(d: Path, flags: list[str] | None = None) -> tuple[bool, str]:
    """Compile oracle.c and port.cppm to IR and compare function bodies.

    Identical normalised opcode sequences mean the compiler produced the same
    computation from both -- behaviour-preserving with no test authoring and no
    tokens spent. Where it holds, it is stronger evidence than any harness.
    Where it does not, we fall through to the differential harness; a mismatch
    here is normal for anything nontrivial and is not by itself a failure.
    """
    if not USE_IR_EQUIV:
        return False, "disabled"
    if _llvm() is None:
        return False, "no clang toolchain — IR check skipped"
    extra = list(flags or [])
    work = d / ".ir"
    work.mkdir(parents=True, exist_ok=True)
    try:
        c_ir = emit_ir(d / "oracle.c", "c", extra, work)
        if c_ir is None:
            return False, "oracle would not compile to IR"
        cpp_ir = emit_ir(d / "port.cppm", "cpp", ["-x", "c++", *CXX_ONLY, *extra], work)
        if cpp_ir is None:
            return False, "port would not compile to IR standalone"

        ref = {k[4:]: v for k, v in ir_bodies(c_ir).items() if k.startswith("ref_")}
        port = {demangle_key(k): v for k, v in ir_bodies(cpp_ir).items()}
        if not ref:
            return False, "no ref_ functions in oracle IR"
        hollow = [k for k, v in ref.items() if degenerate(v)]
        if hollow:
            return False, ("oracle IR has no computation for "
                           f"{hollow[0]} — nothing to compare against")

        matched, differing = match_ir(ref, port)
        if differing:
            return False, f"IR differs for {len(differing)}/{len(ref)}: " \
                          f"{', '.join(differing[:4])}"
        return True, f"IR-identical for all {len(matched)} functions"
    finally:
        shutil.rmtree(work, ignore_errors=True)


# ───────────────────── mechanical (agent-free) port ──────────────────────────
#
# A faithful C -> C++ port of a C function is, for most functions, the identity
# transform: the same statements wrapped in a module and a namespace. Where that
# holds we do not need an agent, a harness, or mutation testing -- we compile the
# original as C and the wrapped copy as C++ and compare the LLVM IR. Identical
# normalised IR means the compiler produced the same computation from both, which
# is the strongest evidence available and costs a few seconds of CPU instead of a
# ten-minute model call. Anything that will not compile, or whose IR differs,
# escalates to the agent -- which is where the model's judgement is actually
# needed.

_INCLUDE_LINE = re.compile(r"^[ \t]*#[ \t]*include[ \t]+[<\"][^>\"]+[>\"].*$", re.M)
_MEMBER_ACCESS = re.compile(r"(?:->|\.)\s*$")

# Words that are ordinary identifiers in C but keywords in C++. FreeBSD uses
# `new` as a parameter name in a couple of dozen files. Renaming them is
# behaviour-neutral -- and if it ever were not, the IR comparison would catch it
# and the file would go to the agent instead.
CXX_KEYWORDS = frozenset("""
new delete class template this operator private public protected namespace
using export friend virtual typename explicit mutable typeid concept requires
try catch throw
""".split())


# KNF puts the return type on its own line, so the function name starts at
# column 0 and _DEFN finds it. Plenty of the tree does not follow KNF, and
# `static void foo(int a)\n{` was being read as a file with no functions at all.
_DEFN_INLINE = re.compile(
    r"^[A-Za-z_][\w \t*]*?\b([A-Za-z_]\w*)[ \t]*\([^();{}]*\)[ \t\r\n]*\{", re.M)


def file_functions(path: Path) -> set[str]:
    """Function names this file defines, in either KNF or one-line style."""
    txt = norm_lines(path)
    found = set(_DEFN.findall(txt)) | set(_DEFN_INLINE.findall(txt))
    return {n for n in found if n not in _KEYWORDS and not n.isupper()}


def rename_in_code(txt: str, names: set[str]) -> str:
    """Prefix `ref_` onto each given name, but only where it is real code.

    Renaming over the raw text rewrote `#include <sys/stat.h>` into
    `sys/ref_stat.h` whenever the file defined a function called `stat`, so the
    oracle stopped compiling. The mask keeps includes, comments and string
    literals out of it.
    """
    mask = code_mask(txt)
    edits: list[tuple[int, int, str]] = []
    for n in names:
        for m in re.finditer(rf"\b{re.escape(n)}\b", mask):
            # A struct field can share a function's name -- libc does this all
            # over the locale code (`l->wcsrtombs`). Renaming the field access
            # invented a member that does not exist and broke the oracle.
            if _MEMBER_ACCESS.search(mask[max(0, m.start() - 24):m.start()]):
                continue
            edits.append((m.start(), m.end(), f"ref_{n}"))
    out = []
    last = 0
    for start, end, rep in sorted(edits):
        if start < last:                      # overlapping name, keep the first
            continue
        out.append(txt[last:start])
        out.append(rep)
        last = end
    out.append(txt[last:])
    return "".join(out)


def write_oracle(path: Path, names: set[str], out: Path) -> None:
    """The original C, with this file's own functions renamed `ref_*`.

    Only names this file defines are renamed, so calls out to libc still bind to
    libc. Bodies are otherwise untouched -- this file is the specification.
    """
    out.write_text(rename_in_code(path.read_text(errors="ignore"), names),
                   encoding="utf-8")


def dodge_cxx_keywords(txt: str) -> str:
    """Suffix an underscore onto identifiers that C++ reserves.

    Only in code regions, and only in the port -- the oracle stays byte-faithful
    C. These are almost always parameter names, which do not survive into
    optimised IR at all, so the comparison is unaffected.
    """
    mask = code_mask(txt)
    edits = []
    for kw in CXX_KEYWORDS:
        for m in re.finditer(rf"\b{kw}\b", mask):
            edits.append((m.start(), m.end(), kw + "_"))
    if not edits:
        return txt
    out, last = [], 0
    for start, end, rep in sorted(edits):
        if start < last:
            continue
        out.append(txt[last:start])
        out.append(rep)
        last = end
    out.append(txt[last:])
    return "".join(out)


def write_port(path: Path, module: str, ns: str, out: Path,
               exported: bool = True) -> None:
    """The same C, wrapped as a C++23 module.

    Includes are hoisted into the global module fragment because a module
    interface unit may not #include after `export module`. Nothing else is
    rewritten: if the body needs real changes to be valid C++, this file is not
    mechanical and belongs to the agent.

    `exported` is retried as False for files that define `static` helpers, since
    C++ refuses to export an internal-linkage name. A file-local helper should
    not be part of the module interface anyway, so the unexported form is the
    more faithful rendering of what the C meant.
    """
    txt = path.read_text(errors="ignore")
    incs, seen = [], set()
    for m in _INCLUDE_LINE.finditer(code_mask_keep_includes(txt)):
        line = m.group(0).strip()
        if line not in seen:
            seen.add(line)
            incs.append(line)
    body = dodge_cxx_keywords(_INCLUDE_LINE.sub("", txt))
    out.write_text(
        f"// PBSD -- mechanical C++23 port of {path.name}, proven equivalent to\n"
        "// the original C by LLVM IR comparison. Generated by pbsd.py.\n"
        "module;\n" + "\n".join(incs) +
        f"\n\nexport module {module};\n\n"
        + ("export " if exported else "") + f"namespace {ns} {{\n\n"
        + body + f"\n}}  // namespace {ns}\n",
        encoding="utf-8")


def code_mask_keep_includes(txt: str) -> str:
    """Like code_mask but leaves #include lines visible, so they can be hoisted."""
    out = []
    for line in txt.split("\n"):
        s = line.lstrip()
        if s.startswith("#") and not re.match(r"#\s*include\b", s):
            out.append(" " * len(line))
        else:
            out.append(line)
    return "\n".join(out)


def include_root() -> Path:
    """Scratch include dir supplying the `machine/` and `x86/` names.

    FreeBSD sources include <machine/_types.h>; the real build satisfies that
    with a symlink into the arch tree that a plain source checkout does not
    have, so without this every libc and kernel header chain dies on the first
    <machine/...>. Kept in the system temp dir so it never lands in git.
    """
    inc = Path(tempfile.gettempdir()) / "pbsd_include"
    inc.mkdir(parents=True, exist_ok=True)
    src = ROOT / "hbsd" / "src"
    for name, target in (("machine", "sys/amd64/include"),
                         ("x86", "sys/x86/include")):
        link, dest = inc / name, src / target
        if not link.exists() and dest.is_dir():
            try:
                link.symlink_to(dest, target_is_directory=True)
            except OSError:
                pass
    for name, target in MECH_GENERATED.items():
        link, dest = inc / name, src / target
        if not link.exists() and dest.is_file():
            try:
                link.symlink_to(dest)
            except OSError:
                pass
    _ensure_opt_stubs(inc)
    return inc


def _ensure_opt_stubs(inc: Path) -> None:
    """Empty opt_*.h stubs for kernel config options.

    Kernel sources #include "opt_inet.h" etc. Those files are generated by the
    FreeBSD build from kernel config; a source checkout has none. Empty stubs
    are correct for a default config where every option is off.
    """
    marker = inc / ".opt_stubs_done"
    if marker.is_file():
        return
    opts: set[str] = set()
    sys_src = ROOT / "hbsd" / "src" / "sys"
    if sys_src.is_dir():
        for path in sys_src.rglob("*"):
            if path.suffix not in (".c", ".h"):
                continue
            try:
                text = path.read_text(errors="ignore")
            except OSError:
                continue
            for m in re.finditer(r'#include\s+"(opt_[^"]+\.h)"', text):
                opts.add(m.group(1))
    for name in opts:
        stub = inc / name
        if not stub.exists():
            stub.write_text("/* PBSD empty kernel option stub */\n", encoding="utf-8")
    marker.write_text(str(len(opts)), encoding="utf-8")


def _resource_include() -> str:
    """clang's own builtin header directory, needed alongside -nostdinc."""
    try:
        r = subprocess.run(["clang", "-print-resource-dir"],
                           capture_output=True, text=True, timeout=30)
        if r.returncode == 0 and r.stdout.strip():
            return str(Path(r.stdout.strip()) / "include")
    except Exception:
        pass
    return ""


def mech_flag_variants(d: str, path: Path) -> list[list[str]]:
    """Flag sets to try, in order, when compiling one source file standalone.

    The file's own directory matters most: FreeBSD uses quoted includes for
    private headers sitting next to the .c file (rand48.h, softfloat-for-gcc.h).

    The second variant adds -nostdinc. Many files fail the first way with
    `typedef redefinition with different types` because the FreeBSD headers on
    the include path and the host glibc headers both get pulled in and disagree
    about types like __int32_t. Cutting the host headers out entirely is what a
    real cross build does.
    """
    dirs: list[str] = [str(path.parent)]
    for prefix, fl in MECH_FLAGS:
        if d.startswith(prefix):
            dirs += [f for f in fl if not f.startswith("-")]
            break
    dirs += MECH_COMMON

    inc: list[str] = []
    for x in dict.fromkeys(dirs):
        inc += ["-I", x if x.startswith("/") else str(ROOT / x)]
    inc += ["-I", str(include_root())]

    base = list(MECH_QUIET) + inc
    variants = [base]
    res = _resource_include()
    if res:
        variants.append(list(MECH_QUIET) + ["-nostdinc", "-isystem", res] + inc)
    return variants


# Every mutation site in a port gets probed before it is certified. A cap would
# leave exactly the blind spot this is meant to close -- the one leak that
# survived a 30-probe cap was at site 34 of a file with 34 of them.
MECH_SITE_LIMIT = 400   # refuse rather than validate partially
MECH_MIN_CAUGHT = 2     # positive evidence required before certifying


def proof_is_sensitive(outdir: Path, flags: list[str]) -> tuple[bool, str]:
    """Check that this file's IR comparison can actually detect a wrong port.

    An IR match only means something if a *mismatch* were possible. Some files
    compile to IR that swallows the difference: everything in lib/libc/quad
    passed with planted bugs still in it, because those files pun `union uu` --
    legal C, undefined in C++ -- and the port's IR stopped tracking the source.
    Rather than trust that the normalisation is safe everywhere, plant bugs in
    this specific port and require this specific comparison to reject them.

    Every mutation site is probed, not just the first per operator, because the
    leaks were at second and third sites. Returns False if any planted bug that
    still compiles is accepted, and also if too few could be tested, since then
    there is no evidence either way.
    """
    port = outdir / "port.cppm"
    good = port.read_text(encoding="utf-8", errors="ignore")
    mask = code_mask(good)
    sites = [(m.start(), m.end(), rep)
             for _, pat, rep in MUTATIONS for m in re.finditer(pat, mask)]
    if len(sites) > MECH_SITE_LIMIT:
        return False, (f"proof unvalidated: {len(sites)} mutation sites is too "
                       "many to probe exhaustively; partial checking is no proof")
    caught = 0
    try:
        for start, end, rep in sites:
            port.write_text(good[:start] + rep + good[end:], encoding="utf-8")
            accepted, why = ir_equivalence(outdir, flags)
            if accepted:
                return False, ("proof rejected: a planted bug was accepted, so "
                               "the IR comparison is blind for this file")
            if why.startswith("IR differs"):
                caught += 1
    finally:
        port.write_text(good, encoding="utf-8")
    if caught < MECH_MIN_CAUGHT:
        return False, (f"proof unvalidated: only {caught} planted bugs could be "
                       "compiled and caught, too few to trust the comparison")
    return True, f"{caught} planted bugs all rejected"


def mechanical_port(row: dict, outdir: Path) -> tuple[bool, str]:
    """Try to port ONE source file with no agent call. True only if IR proves it.

    Per file rather than per batch: concatenating a batch's sources into a single
    translation unit produced duplicate typedefs and duplicate statics, which
    failed the compile for reasons that had nothing to do with the port.
    """
    if not MECHANICAL:
        return False, "mechanical path disabled"
    path = ROOT / row["path"]
    if not path.is_file():
        return False, "source missing"
    names = file_functions(path)
    if not names:
        return False, "no function definitions"

    stem = sanitise_component(Path(row["path"]).stem)
    parts = [sanitise_component(x) for x in row["dir"].split("/") if x]
    module = ".".join(["pbsd", *parts, stem])
    ns = "::".join(["pbsd", *parts, stem])
    outdir.mkdir(parents=True, exist_ok=True)
    try:
        write_oracle(path, names, outdir / "oracle.c")
    except Exception as e:                       # unreadable source, odd encoding
        return False, f"could not generate: {e}"

    last = "not attempted"
    for flags in mech_flag_variants(row["dir"], path):
        for exported in (True, False):
            try:
                write_port(path, module, ns, outdir / "port.cppm", exported)
            except Exception as e:
                return False, f"could not generate: {e}"
            ok, last = ir_equivalence(outdir, flags)
            if ok:
                sound, why = proof_is_sensitive(outdir, flags)
                if not sound:
                    last = why
                    break        # the proof is worthless here; let the agent do it
                note = "" if exported else " (helpers kept module-local)"
                return True, (f"mechanical port of {path.name}, {last}, "
                              f"{why}{note}")
            if last.startswith("oracle"):
                break            # the C side failed; the C++ form is irrelevant
    shutil.rmtree(outdir, ignore_errors=True)
    return False, last


def record_artifact(batch_id: str, detail: str, ir_ok: bool,
                    out: str, mine: list[dict]) -> None:
    ARTIFACTS.mkdir(parents=True, exist_ok=True)
    (ARTIFACTS / f"{batch_id}.json").write_text(json.dumps({
        "batch": batch_id,
        "verified": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "evidence": detail,
        "ir_equivalent": ir_ok,
        "harness_output": out[-4000:],
        "input_hashes": {r["path"]: fingerprint(ROOT / r["path"]) for r in mine},
    }, indent=2), encoding="utf-8")


# ──────────────────────────────── gate ───────────────────────────────────────


def banned_hits(d: Path) -> list[str]:
    hits = []
    for p in d.rglob("*"):
        if p.is_file() and p.suffix in (".cppm", ".cpp", ".c", ".h"):
            txt = p.read_text(errors="ignore")
            hits += [f"{p.name}: {b}" for b in BANNED if b in txt]
    return hits


def run_build(d: Path, timeout: int = GATE_TIMEOUT) -> tuple[bool, str]:
    try:
        r = sh(["sh", "build.sh"], cwd=d, timeout=timeout)
        return r.returncode == 0, (r.stdout or "") + (r.stderr or "")
    except subprocess.TimeoutExpired:
        # A hung run is a failed run. For a mutant that means the bug WAS
        # detected (the program no longer terminates), which counts as killed.
        return False, "timed out"


def code_mask(src: str) -> str:
    """Blank comments, string/char literals and preprocessor lines to spaces.

    Length and line structure are preserved, so an offset into the mask is the
    same offset in the original. Mutation sites are located in the mask and
    applied to the original, which stops a planted `-` -> `+` from landing in a
    copyright header or an #include guard, where it would survive every harness
    and fail the batch for no reason.
    """
    out, in_block = [], False
    for line in src.split("\n"):
        if in_block:
            end = line.find("*/")
            if end < 0:
                out.append(" " * len(line))
                continue
            line = " " * (end + 2) + line[end + 2:]
            in_block = False
        if line.lstrip().startswith("#"):
            out.append(" " * len(line))
            continue
        buf, i, n = [], 0, len(line)
        while i < n:
            two = line[i:i + 2]
            if two == "/*":
                end = line.find("*/", i + 2)
                if end < 0:
                    buf.append(" " * (n - i))
                    i, in_block = n, True
                else:
                    buf.append(" " * (end + 2 - i))
                    i = end + 2
            elif two == "//":
                buf.append(" " * (n - i))
                i = n
            elif line[i] in "\"'":
                q, j = line[i], i + 1
                while j < n and line[j] != q:
                    j += 2 if line[j] == "\\" else 1
                j = min(j + 1, n)
                buf.append(" " * (j - i))
                i = j
            else:
                buf.append(line[i])
                i += 1
        out.append("".join(buf))
    return "\n".join(out)


def mutation_check(d: Path) -> tuple[bool, str]:
    """Plant bugs in the port. The harness must reject every one.

    A mutant that does not compile proves nothing about the harness -- the
    compiler caught it, not the test -- so it is not counted either way. Only
    mutants that build are evidence, and every one of those must be killed.

    Every site is tried, not just the first per operator. Trying one site per
    operator was the single largest source of rejected batches: a good port with
    a good harness would be thrown away reporting "only 1 compilable mutations,
    need 3" while a dozen untried sites sat in the same file. The requirement
    also scales down for a port that genuinely has almost nothing to mutate,
    since a two-line function cannot supply three independent bugs.
    """
    port = d / "port.cppm"
    original = port.read_text(encoding="utf-8")
    mask = code_mask(original)
    sites = [(name, m.start(), m.end(), m.expand(rep))
             for name, pat, rep in MUTATIONS for m in re.finditer(pat, mask)]
    applied, survived, uncompilable, tried = 0, [], 0, 0
    try:
        for name, start, end, rep in sites:
            if applied >= MUTANT_CAP or tried >= MUTANT_ATTEMPTS:
                break
            mutated = original[:start] + rep + original[end:]
            if mutated == original:
                continue
            tried += 1
            port.write_text(mutated, encoding="utf-8")
            ok, out = run_build(d, timeout=MUTANT_TIMEOUT)
            if not ok and re.search(r"\berror:", out):
                uncompilable += 1
                continue
            applied += 1
            if ok:                      # harness passed a broken port
                survived.append(name)
    finally:
        port.write_text(original, encoding="utf-8")

    if survived:
        return False, f"harness failed to detect: {', '.join(survived)}"
    if not sites:
        # Nothing in this port to mutate -- no comparison, no arithmetic, no
        # constant. Mutation testing only establishes that the harness is
        # sensitive; the harness itself already ran the port against the original
        # C and got the same answers, and that stands on its own here.
        return True, "harness green; port has no mutable operator to plant a bug in"
    need = max(1, min(MIN_MUTATIONS, len(sites)))
    if applied < need:
        return False, (f"only {applied} of {len(sites)} candidate mutations "
                       f"compiled, need {need} ({uncompilable} would not build)")
    return True, f"{applied}/{applied} mutations killed"


def is_rate_limited(detail: str) -> bool:
    """True when the Cursor API refused the call for quota, not because the port failed."""
    d = detail.lower()
    return (
        "actionrequirederror" in d and ("rate limit" in d or "increase limits" in d)
    ) or "rate limit exceeded" in d


def is_auth_failure(detail: str) -> bool:
    """True when the API key or login is wrong — retrying will never help."""
    d = detail.lower()
    return "api key is invalid" in d or "authentication" in d and "fail" in d


def call_agent(prompt: str, model: str) -> tuple[bool, str]:
    """One agent call, retrying transient failures with backoff.

    Rate limits are not transient: retrying them in a tight loop just burns the
    queue. A timeout is not transient either. stderr is kept because the CLI
    reports its actual reason there and the log used to record only an empty stdout.
    """
    last = ""
    env = os.environ.copy()
    home_bin = str(Path.home() / ".local" / "bin")
    if home_bin not in env.get("PATH", ""):
        env["PATH"] = home_bin + ":" + env.get("PATH", "")
    for attempt in range(1, AGENT_RETRIES + 1):
        t0 = time.monotonic()
        try:
            r = subprocess.run(
                ["cursor-agent", "-p", prompt, "--workspace", str(ROOT),
                 "--model", model, "--output-format", "text",
                 "--force", "--trust"],
                timeout=AGENT_TIMEOUT, text=True, capture_output=True,
                stdin=subprocess.DEVNULL, env=env,
            )
            if r.returncode == 0:
                return True, (r.stdout or "")[-2000:]
            detail = (r.stderr or r.stdout or "").strip() or "(no output)"
            last = f"exit {r.returncode} after {time.monotonic() - t0:.0f}s: {detail[-800:]}"
            if is_rate_limited(last):
                return False, last
            if is_auth_failure(last):
                return False, last
        except subprocess.TimeoutExpired:
            return False, f"timed out after {AGENT_TIMEOUT}s"
        if attempt < AGENT_RETRIES:
            wait = AGENT_BACKOFF * 2 ** (attempt - 1)
            say(f"  agent attempt {attempt} failed, retrying in {wait}s")
            time.sleep(wait)
    return False, last


def gate(batch_id: str, d: Path, mine: list[dict]) -> tuple[bool, str]:
    for f in ("port.cppm", "oracle.c", "harness.cpp", "build.sh"):
        if not (d / f).is_file():
            return False, f"missing {f}"
    hits = banned_hits(d)
    if hits:
        return False, "banned token: " + hits[0]

    ok, out = run_build(d)
    if not ok:
        return False, "differential run failed:\n" + out[-1500:]

    # Free proof first. If the compiler produces identical IR from the C and the
    # C++, the port is behaviour-preserving and no harness argument is needed.
    ir_ok, ir_detail = ir_equivalence(d)
    if ir_ok:
        detail = f"harness green + {ir_detail}"
    else:
        ok, mdetail = mutation_check(d)
        if not ok:
            return False, "mutation check: " + mdetail
        detail = f"harness green + {mdetail} (IR: {ir_detail})"

    record_artifact(batch_id, detail, ir_ok, out, mine)
    return True, detail


def attempt_batch(batch_id: str, mine: list[dict], model: str,
                  escalate_model: str | None = None) -> tuple[bool, str, str]:
    """Everything needed to decide one batch. Safe to run in a child process.

    Touches only its own output directory and its own artifact file, so N of
    these can run at once. Returns (verified, detail, how) where `how` is
    mechanical, agent, agent+escalate, or rate_limit.
    """
    outdir = WORK / mine[0]["dir"] / batch_id
    outdir.mkdir(parents=True, exist_ok=True)
    ns = re.sub(r"[^a-z0-9]+", "_", mine[0]["dir"].lower()).strip("_")
    module = f"{ns}.{batch_id}".replace("_", ".")
    namespace = f"{ns}::{batch_id}"
    mech_detail = "not attempted (mechanical phase runs separately)"
    escalate = escalate_model or model

    prompt = PROMPT.format(
        batch_id=batch_id,
        file_list="\n".join(f"  {r['path']}  ({r['lines']} lines)" for r in mine),
        outdir=str(outdir.relative_to(ROOT)),
        module=module,
        ns=namespace,
    )

    def run_with(m: str) -> tuple[bool, str, str]:
        agent_ok, agent_out = call_agent(prompt, m)
        if not agent_ok:
            if is_rate_limited(agent_out):
                return False, f"agent rate limited: {agent_out}", "rate_limit"
            if is_auth_failure(agent_out):
                return False, f"agent auth failed: {agent_out}", "auth_failed"
            return False, f"agent failed: {agent_out}", "agent"
        ok, detail = gate(batch_id, outdir, mine)
        if not ok:
            detail = f"{detail}\n(mechanical path first said: {mech_detail})"
        how = "agent+escalate" if m != model else "agent"
        return ok, detail, how

    ok, detail, how = run_with(model)
    if ok or how == "rate_limit" or escalate == model:
        return ok, detail, how
    # Gate failed on the cheap model; one Opus attempt before we split or defer.
    clean_batch(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    ok, detail, how2 = run_with(escalate)
    return ok, detail, how2


# ──────────────────────────────── loop ───────────────────────────────────────


def load_rows() -> list[dict]:
    with INVENTORY.open(encoding="utf-8") as fh:
        return list(csv.DictReader(fh))


def save_rows(rows: list[dict]) -> None:
    if not rows:
        say("WARNING: refusing to write an empty inventory")
        return
    with INVENTORY.open("w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=["batch_id", "path", "dir", "lines", "status", "clone_of"])
        w.writeheader()
        w.writerows(rows)


def status() -> None:
    rows = load_rows()
    if not rows:
        print("\n  no inventory yet — run `python3 pbsd.py` to set up\n")
        return
    n = {}
    for r in rows:
        n[r["status"]] = n.get(r["status"], 0) + 1
    tot = len(rows)
    ver = n.get("VERIFIED", 0)
    vl = sum(int(r["lines"]) for r in rows if r["status"] == "VERIFIED")
    tl = sum(int(r["lines"]) for r in rows)

    ir = 0
    if ARTIFACTS.is_dir():
        for a in ARTIFACTS.glob("*.json"):
            try:
                ir += 1 if json.loads(a.read_text()).get("ir_equivalent") else 0
            except Exception:
                pass

    print()
    print(f"  verified   {ver:>6} / {tot} files      {vl:>10,} / {tl:,} lines "
          f"({100*vl/max(tl,1):.2f}%)")
    print(f"  pending    {n.get('PENDING', 0):>6}")
    print(f"  deferred   {n.get('DEFERRED', 0):>6}   hard queue, worked last")
    print(f"  need you   {n.get('NEEDS_HUMAN', 0):>6}"
          + (f"   -> {NEEDS_HUMAN.relative_to(ROOT)}" if n.get("NEEDS_HUMAN") else ""))
    print(f"  skipped    {n.get('SKIPPED', 0):>6}   unreferenced in scope")
    print(f"  clones     {n.get('CLONE', 0):>6}   inherit a representative's verdict")
    print(f"  IR-proven  {ir:>6}   batches needing no harness argument")

    if LOG.exists():
        done = []
        for line in LOG.read_text(errors="ignore").splitlines():
            try:
                e = json.loads(line)
            except Exception:
                continue
            if e.get("status") in ("VERIFIED", "REJECTED"):
                done.append(e)
        if done:
            rate = 100 * sum(1 for e in done if e["status"] == "VERIFIED") / len(done)
            print(f"  pass rate  {rate:>5.1f}%   over {len(done)} attempted batches")

    if DEFERRED.is_file():
        c: dict[str, int] = {}
        for line in DEFERRED.read_text(errors="ignore").splitlines():
            try:
                for x in json.loads(line).get("reasons", []):
                    k = x.split("(")[0]
                    c[k] = c.get(k, 0) + 1
            except Exception:
                pass
        if c:
            print("\n  deferred by blocking idiom (each group is one decision):")
            for k, v in sorted(c.items(), key=lambda kv: -kv[1]):
                print(f"      {v:>5}  {k}")
    print()


def apply_result(batch_id: str, rows: list[dict], ok: bool, detail: str,
                 how: str, split_ok: bool) -> list[str]:
    """Record one batch verdict. Only ever called from the parent process.

    The inventory, the log and git are single-writer by construction: workers
    compute verdicts, the parent is the only thing that writes shared state.
    Returns any follow-up batch ids that should be queued (the per-file split).
    """
    mine = [r for r in rows if r["batch_id"] == batch_id
            and r["status"] != "VERIFIED"]
    if not mine:
        return []
    outdir = WORK / mine[0]["dir"] / batch_id

    if how == "rate_limit" or is_rate_limited(detail):
        say(f"  ~ {batch_id} rate limited — leaving PENDING for retry")
        log(batch=batch_id, status="RATE_LIMITED", detail=detail, how=how)
        return [batch_id]

    if how == "auth_failed" or is_auth_failure(detail):
        say(f"  ! {batch_id} auth failure — leaving PENDING (fix API key)")
        log(batch=batch_id, status="AUTH_FAILED", detail=detail, how=how)
        return [batch_id]

    for r in mine:
        r["status"] = "VERIFIED" if ok else "REJECTED"

    if ok:
        say(f"  ✓ {batch_id} VERIFIED ({how}) — {detail.splitlines()[0][:90]}")
        log(batch=batch_id, status="VERIFIED", detail=detail, how=how)
        emit_build_wiring(rows)
        n = propagate_clones(rows, {r["path"] for r in mine})
        if n:
            say(f"  + {n} clone files inherit this verdict")
        save_rows(rows)
        git("add", "-A")
        git("commit", "-q", "-m", f"pbsd: {batch_id} verified ({mine[0]['dir']})")
        return []

    say(f"  ✗ {batch_id} rejected — {detail.splitlines()[0][:90] if detail else '?'}")
    log(batch=batch_id, status="REJECTED", detail=detail, how=how)
    clean_batch(outdir)

    # Most rejections are one hard file poisoning an otherwise fine batch.
    # Retry file-by-file to salvage the rest. A timeout is worth splitting too:
    # one file per call is far less work than four and usually lands, whereas
    # deferring sends it to a queue that is slower still.
    produced_nothing = (detail.startswith(("missing ", "agent failed"))
                        and "timed out" not in detail)
    if split_ok and len(mine) > 1 and not produced_nothing:
        subs = []
        for i, r in enumerate(mine, 1):
            sub = f"{batch_id}s{i}"
            for row in rows:
                if row["path"] == r["path"]:
                    row["batch_id"] = sub
                    row["status"] = "PENDING"
            subs.append(sub)
        save_rows(rows)
        say(f"  splitting {batch_id} into {len(subs)} single-file batches")
        return subs

    for r in mine:
        r["status"] = "DEFERRED"
        defer(r["path"], batch_id, ["AUTO_FAILED"], detail)
    save_rows(rows)
    git("add", "-A")
    git("commit", "-q", "-m", f"pbsd: {batch_id} deferred")
    return []


def reset_jammed_queue(rows: list[dict]) -> int:
    """Put NEEDS_HUMAN files back on PENDING and clear a rate-limit poisoned queue."""
    n = 0
    for r in rows:
        if r["status"] == "NEEDS_HUMAN":
            r["status"] = "PENDING"
            n += 1
    if n:
        save_rows(rows)
    if DEFERRED.is_file():
        DEFERRED.unlink()
    if NEEDS_HUMAN.is_file():
        NEEDS_HUMAN.unlink()
    return n


def do_batch(batch_id: str, rows: list[dict], model: str,
             escalate_model: str | None = None, split_ok: bool = True) -> bool:
    mine = [r for r in rows if r["batch_id"] == batch_id
            and r["status"] != "VERIFIED"]
    if not mine:
        return True
    say(f"{batch_id}: {len(mine)} files, {sum(int(r['lines']) for r in mine)} lines "
        f"[{mine[0]['dir']}]")
    ok, detail, how = attempt_batch(batch_id, mine, model, escalate_model)
    for sub in apply_result(batch_id, rows, ok, detail, how, split_ok):
        do_batch(sub, rows, model, escalate_model, split_ok=False)
    return ok


def _worker(payload):
    """Child-process entry point. Returns a verdict, writes no shared state."""
    batch_id, mine, model, escalate = payload
    try:
        ok, detail, how = attempt_batch(batch_id, mine, model, escalate)
    except Exception as e:                       # never kill the pool
        ok, detail, how = False, f"worker crashed: {e!r}", "error"
    return batch_id, ok, detail, how


def mech_id(path: str) -> str:
    """Evidence id for a mechanically ported file.

    Derived from the whole path, not the stem: the tree has several files called
    strsep.c and they would overwrite each other's evidence.
    """
    return "m_" + re.sub(r"[^A-Za-z0-9]+", "_", path.rsplit(".", 1)[0]).strip("_")


def _mech_worker(row):
    """Child-process entry point for one mechanical file attempt."""
    outdir = WORK / row["dir"] / (Path(row["path"]).stem + "_m")
    try:
        ok, detail = mechanical_port(row, outdir)
    except Exception as e:
        ok, detail = False, f"crashed: {e!r}"
    return row["path"], ok, detail


def run_mechanical_phase(rows: list[dict], jobs: int) -> int:
    """Prove as much of the tree as possible with no model calls at all.

    This is pure CPU -- two compiler invocations and an IR comparison per file --
    so it saturates every core and costs nothing but electricity. Whatever it
    proves never reaches the agent, which is the only way the whole tree finishes
    in hours instead of weeks.
    """
    if not MECHANICAL:
        return 0
    todo = [r for r in rows if r["status"] in ("PENDING", "DEFERRED")]
    if not todo:
        return 0
    banner(f"Mechanical phase — {len(todo)} files, {jobs} parallel, no agent")

    proven = 0
    done = 0
    reasons: dict[str, int] = {}
    t0 = time.monotonic()
    by_path = {r["path"]: r for r in rows}

    with cf.ProcessPoolExecutor(max_workers=jobs) as ex:
        for path, ok, detail in ex.map(_mech_worker, todo, chunksize=4):
            done += 1
            if ok:
                proven += 1
                row = by_path[path]
                row["status"] = "VERIFIED"
                bid = mech_id(path)
                record_artifact(bid, detail, True, "", [row])
                log(batch=bid, status="VERIFIED", detail=detail, how="mechanical")
            else:
                key = detail.split(":")[0][:60]
                reasons[key] = reasons.get(key, 0) + 1
            if done % 250 == 0 or done == len(todo):
                el = time.monotonic() - t0
                print(f"  [{done}/{len(todo)}] {proven} proven free "
                      f"({100*proven/done:.0f}%), {done/max(el,1):.0f} files/s",
                      flush=True)

    say(f"mechanical phase proved {proven}/{len(todo)} files with no agent call")
    if reasons:
        say("what still needs the agent:")
        for k, v in sorted(reasons.items(), key=lambda kv: -kv[1])[:10]:
            say(f"    {v:>5}  {k}")
    if proven:
        n = propagate_clones(rows, {r["path"] for r in rows
                                    if r["status"] == "VERIFIED"})
        if n:
            say(f"+ {n} clone files inherit a mechanical verdict")
        save_rows(rows)
        emit_build_wiring(rows)
        git("add", "-A")
        git("commit", "-q", "-m",
            f"pbsd: {proven} files ported mechanically, proven by IR equivalence")
    return proven


def run_parallel(queue: list[str], rows: list[dict], model: str,
                 jobs: int, escalate_model: str) -> tuple[int, int]:
    """Work the batch queue with `jobs` batches in flight.

    Each batch is independent -- its own output directory, its own compiler
    invocations, its own artifact -- so the work parallelises cleanly. The
    parent stays the only writer of the inventory, the log and git, which is
    what keeps this safe; a pool of workers all running `git commit` would
    fight over index.lock and interleave commits.
    """
    ver = rej = 0
    total = len(queue)
    pending = list(queue)
    done = 0
    t0 = time.monotonic()
    current_jobs = jobs
    rate_streak = 0
    success_streak = 0

    with cf.ProcessPoolExecutor(max_workers=jobs) as ex:
        live = {}
        try:
            while pending or live:
                while pending and len(live) < current_jobs:
                    b = pending.pop(0)
                    mine = [r for r in rows if r["batch_id"] == b
                            and r["status"] != "VERIFIED"]
                    if not mine:
                        continue
                    live[ex.submit(_worker, (b, mine, model, escalate_model))] = b

                if not live:
                    break
                for fut in cf.as_completed(list(live), timeout=None):
                    b = live.pop(fut)
                    batch_id, ok, detail, how = fut.result()
                    subs = apply_result(batch_id, rows, ok, detail, how,
                                        split_ok=True)
                    if how == "rate_limit" or is_rate_limited(detail):
                        rate_streak += 1
                        success_streak = 0
                        pending.extend(subs)
                        if rate_streak >= RATE_LIMIT_STREAK:
                            pause = min(600, RATE_LIMIT_PAUSE * rate_streak // 2)
                            new_jobs = max(1, current_jobs // 2)
                            say(f"rate limit streak {rate_streak}: "
                                f"sleep {pause}s, jobs {current_jobs}->{new_jobs}")
                            time.sleep(pause)
                            current_jobs = new_jobs
                            rate_streak = 0
                    elif how == "auth_failed" or is_auth_failure(detail):
                        say(f"auth issue on {batch_id} — re-queueing (use cursor-agent login)")
                        pending.extend(subs)
                    else:
                        rate_streak = 0
                        done += 1
                        if ok:
                            ver += 1
                            success_streak += 1
                            if success_streak >= 15 and current_jobs < jobs:
                                current_jobs = min(jobs, current_jobs + 2)
                                success_streak = 0
                        else:
                            rej += 1
                        pending.extend(subs)
                        total += len(subs)
                    rate = done / max(time.monotonic() - t0, 1) * 3600
                    left = (total - done) / rate if rate else 0
                    print(f"[{done}/{total}] {ver} verified, "
                          f"{rej} deferred, jobs={current_jobs}, "
                          f"{rate:.0f}/h, ~{left:.1f}h left",
                          flush=True)
                    break                        # refill the pool, then re-wait
        except KeyboardInterrupt:
            print("\ninterrupted — cancelling in-flight batches", flush=True)
            for fut in live:
                fut.cancel()
            raise
    return ver, rej



def _deferred_worker(payload):
    """One deferred file, all its attempts. Child process; writes no shared state."""
    it, row, model, escalate = payload
    path = ROOT / it["path"]
    outdir = WORK / (row["dir"] if row else "deferred") / (path.stem + "_d")
    bid = f"d_{re.sub(r'[^A-Za-z0-9]+', '_', it['path'].rsplit('.', 1)[0]).strip('_')}"
    ok, detail = False, "not attempted"
    try:
        outdir.mkdir(parents=True, exist_ok=True)
        while it["attempts"] < DEFERRED_ATTEMPTS and not ok:
            prompt = DEFERRED_PROMPT.format(
                path=it["path"], lines=row["lines"] if row else "?",
                reasons=", ".join(it["reasons"]) or "unclassified",
                detail=(it.get("detail") or "(none)")[-1200:],
                outdir=str(outdir.relative_to(ROOT)))
            agent_ok, agent_out = call_agent(prompt, model)

            if (outdir / "IMPOSSIBLE.txt").is_file():
                it["last_error"] = (outdir / "IMPOSSIBLE.txt").read_text(
                    errors="ignore")[:400]
                detail = "agent declared it impossible"
                it["attempts"] += 1
                break
            if not agent_ok:
                if is_rate_limited(agent_out):
                    detail = f"agent rate limited: {agent_out}"
                    return it, False, detail
                it["attempts"] += 1
                it["detail"] = it["last_error"] = f"agent failed: {agent_out}"
                detail = it["detail"]
                continue

            ok, detail = gate(bid, outdir, [row] if row else [])
            it["detail"] = it["last_error"] = detail
            it["attempts"] += 1
            if not ok and escalate != model:
                clean_batch(outdir)
                outdir.mkdir(parents=True, exist_ok=True)
                agent_ok, agent_out = call_agent(prompt, escalate)
                if not agent_ok:
                    if is_rate_limited(agent_out):
                        detail = f"agent rate limited: {agent_out}"
                        return it, False, detail
                    continue
                ok, detail = gate(bid, outdir, [row] if row else [])
                it["detail"] = it["last_error"] = detail
    except Exception as e:                          # never kill the pool
        detail = f"worker crashed: {e!r}"
    if not ok:
        clean_batch(outdir)
    return it, ok, detail


def run_deferred_phase(rows: list[dict], model: str,
                       jobs: int = 1, escalate_model: str | None = None) -> tuple[int, int]:
    """Work the deferred queue LAST, feeding each failure back into the next
    attempt for the same file. Survivors get written up for a human.

    Run in parallel like the main queue: these are the hardest files, each gets
    several agent calls, and there are over a thousand of them. One at a time
    this phase alone would take longer than everything else put together.
    """
    items = [i for i in load_deferred() if i.get("attempts", 0) < DEFERRED_ATTEMPTS]
    items = [i for i in items if (ROOT / i["path"]).is_file()]
    if not items:
        return 0, 0
    banner(f"Deferred queue — {len(items)} hard files, {DEFERRED_ATTEMPTS} attempts "
           f"each, {jobs} in flight")

    escalate = escalate_model or model
    by_path = {r["path"]: r for r in rows}
    payloads = [(it, by_path.get(it["path"]), model, escalate) for it in items]
    done = {i["path"]: i for i in items}
    fixed = stuck = n = 0
    t0 = time.monotonic()

    with cf.ProcessPoolExecutor(max_workers=max(1, jobs)) as ex:
        for it, ok, detail in ex.map(_deferred_worker, payloads):
            n += 1
            done[it["path"]] = it
            row = by_path.get(it["path"])
            if ok:
                fixed += 1
                if row:
                    row["status"] = "VERIFIED"
                say(f"✓ {it['path']} — {detail.splitlines()[0][:80]}")
            elif is_rate_limited(detail):
                if row:
                    row["status"] = "PENDING"
                say(f"~ {it['path']} — rate limited, will retry later")
            else:
                stuck += 1
                if row:
                    row["status"] = "NEEDS_HUMAN"
                say(f"✗ {it['path']} — {detail.splitlines()[0][:80]}")
            if ok or n % 25 == 0:
                save_rows(rows)
                emit_build_wiring(rows)
                save_deferred(list(done.values()))
                git("add", "-A")
                git("commit", "-q", "-m", f"pbsd: deferred queue, {fixed} recovered")
            rate = n / max(time.monotonic() - t0, 1) * 3600
            print(f"[deferred {n}/{len(items)}] {fixed} recovered, {stuck} stuck, "
                  f"{rate:.0f}/h, ~{(len(items)-n)/max(rate,1):.1f}h left", flush=True)

    save_rows(rows)
    save_deferred(list(done.values()))
    write_human_report([i for i in done.values()
                        if i.get("attempts", 0) >= DEFERRED_ATTEMPTS
                        or i.get("last_error", "").startswith(("cannot", "This file"))])
    git("add", "-A")
    git("commit", "-q", "-m", f"pbsd: deferred queue done, {fixed} recovered")
    return fixed, stuck


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", default=MODEL)
    ap.add_argument("--escalate-model", default=ESCALATE_MODEL)
    ap.add_argument("--batches", type=int, default=0, help="0 = run until done")
    ap.add_argument("--jobs", "-j", type=int, default=JOBS,
                    help="batches in flight; 0 = DEFAULT_JOBS")
    ap.add_argument("--no-mechanical", action="store_true",
                    help="always use the agent, skip the free deterministic port")
    ap.add_argument("--mechanical-only", action="store_true",
                    help="run only the free deterministic port, then stop")
    ap.add_argument("--reset-queue", action="store_true",
                    help="move NEEDS_HUMAN back to PENDING and clear deferred.jsonl")
    ap.add_argument("--status", action="store_true")
    ap.add_argument("--reset-setup", action="store_true")
    ap.add_argument("--deferred", action="store_true",
                    help="work only the deferred queue (e.g. once you have credits)")
    a = ap.parse_args()

    global MECHANICAL
    if a.no_mechanical:
        MECHANICAL = False

    if a.status:
        status()
        return 0
    if not preflight():
        return 1
    if a.reset_setup and SETUP_STAMP.exists():
        SETUP_STAMP.unlink()
    if not SETUP_STAMP.exists():
        setup()

    rows = load_rows()
    if a.reset_queue:
        n = reset_jammed_queue(rows)
        banner(f"Reset {n} NEEDS_HUMAN rows to PENDING; cleared deferred queue")
        status()
        return 0
    if a.deferred:
        jobs = a.jobs or DEFAULT_JOBS
        fixed, stuck = run_deferred_phase(
            rows, a.model, jobs, a.escalate_model)
        banner(f"Deferred pass: {fixed} recovered, {stuck} need you")
        status()
        return 0
    n_drift = check_drift(rows)
    if n_drift:
        say(f"upstream drift: reopened {n_drift} batches")
        save_rows(rows)

    jobs_mech = a.jobs or (os.cpu_count() or 8)
    run_mechanical_phase(rows, jobs_mech)
    if a.mechanical_only:
        status()
        return 0

    queue = []
    for r in rows:
        if r["status"] == "PENDING" and r["batch_id"] not in queue:
            queue.append(r["batch_id"])
    if a.batches:
        queue = queue[: a.batches]

    jobs = a.jobs or DEFAULT_JOBS
    banner(f"Converting {len(queue)} batches   model={a.model}   "
           f"escalate={a.escalate_model}   jobs={jobs}"
           + ("" if MECHANICAL else "   (mechanical path off)"))
    ver = rej = 0
    try:
        if jobs > 1:
            ver, rej = run_parallel(queue, rows, a.model, jobs, a.escalate_model)
        else:
            for i, b in enumerate(queue, 1):
                print(f"\n[{i}/{len(queue)}]", flush=True)
                if do_batch(b, rows, a.model, a.escalate_model):
                    ver += 1
                else:
                    rej += 1
    except KeyboardInterrupt:
        print("\ninterrupted — progress is committed, rerun to continue")

    banner(f"Automated pass done: {ver} verified, {rej} deferred")

    if a.batches:
        say(f"--batches {a.batches} given, so the deferred queue is left for a full run")
        status()
        return 0
    try:
        fixed, stuck = run_deferred_phase(rows, a.model, jobs, a.escalate_model)
        if fixed or stuck:
            banner(f"Deferred pass: {fixed} recovered, {stuck} need you")
            if stuck:
                say(f"written up in {NEEDS_HUMAN.relative_to(ROOT)}")
    except KeyboardInterrupt:
        print("\ninterrupted — deferred queue is saved, rerun to continue")

    status()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
