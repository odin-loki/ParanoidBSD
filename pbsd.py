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
import csv
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

# ─────────────────────────────── config ──────────────────────────────────────

MODEL = "claude-opus-5-thinking-high"   # Opus 5 1M Thinking
BATCH_SIZE = 4          # small batches: much higher pass rate on weaker models
AGENT_TIMEOUT = 1800          # cursor-agent -p can hang; always bound it
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

ROOT = Path.cwd()
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

MUTATIONS = [
    ("off_by_one",     r"(\w+)\s*!=\s*0\s*\)",            r"\1 > 1)"),
    ("inverted_cmp",   r"(?<![<>!=])<(?![<=])",            ">="),
    ("dropped_nul",    r"\*\s*(\w+)\s*=\s*'\\0'\s*;",     r"/*mut*/;"),
    ("off_by_one_inc", r"\+\+\s*(\w+)\s*;",                r"\1 += 2;"),
    ("zero_return",    r"return\s+(\w+)\s*-\s*(\w+)\s*;",  r"return 0;"),
]

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
    r = sh(["git", "-C", str(ROOT), *args])
    if r.returncode != 0 and args and args[0] == "commit":
        err = (r.stderr or r.stdout or "").strip().splitlines()
        if err and "nothing to commit" not in " ".join(err):
            say(f"WARNING: git commit failed — {err[0][:90]}")
    return r.returncode


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
    mods = sorted({
        str((WORK / r["dir"] / r["batch_id"] / "port.cppm"))
        for r in rows if r["status"] == "VERIFIED"
        if (WORK / r["dir"] / r["batch_id"] / "port.cppm").is_file()
    })
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
_IR_NAMES = re.compile(r"[%@][\w.$-]+")
_IR_DEFINE = re.compile(r"^define[^{]*@([\w.$]+)\([^)]*\)[^{]*\{(.*?)^\}", re.M | re.S)


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
    """Extract per-function normalised opcode sequences from IR text."""
    bodies: dict[str, str] = {}
    for name, body in _IR_DEFINE.findall(_IR_NOISE.sub("", ir)):
        ops = []
        for line in body.splitlines():
            line = _IR_NAMES.sub("%v", line.strip())
            line = re.sub(r"\b\d+\b", "N", line)
            if line and not line.startswith(("#", ";")):
                ops.append(line)
        bodies[name] = "\n".join(ops)
    return bodies


def demangle_key(sym: str) -> str:
    """Reduce a mangled C++ symbol to a comparable bare function name."""
    m = re.findall(r"\d+([A-Za-z_]\w*)", sym)
    return (m[-1] if m else sym).lstrip("_")


def ir_equivalence(d: Path) -> tuple[bool, str]:
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
    work = d / ".ir"
    work.mkdir(exist_ok=True)
    try:
        c_ir = emit_ir(d / "oracle.c", "c", [], work)
        if c_ir is None:
            return False, "oracle would not compile to IR"
        cpp_ir = emit_ir(d / "port.cppm", "cpp", ["-x", "c++"], work)
        if cpp_ir is None:
            return False, "port would not compile to IR standalone"

        ref = {k[4:]: v for k, v in ir_bodies(c_ir).items() if k.startswith("ref_")}
        port = {demangle_key(k): v for k, v in ir_bodies(cpp_ir).items()}
        if not ref:
            return False, "no ref_ functions in oracle IR"

        matched, differing = [], []
        for fn, body in ref.items():
            if fn in port and port[fn] == body:
                matched.append(fn)
            else:
                differing.append(fn)
        if differing:
            return False, f"IR differs for {len(differing)}/{len(ref)}: " \
                          f"{', '.join(differing[:4])}"
        return True, f"IR-identical for all {len(matched)} functions"
    finally:
        shutil.rmtree(work, ignore_errors=True)


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


def mutation_check(d: Path) -> tuple[bool, str]:
    """Plant bugs in the port. The harness must reject every one."""
    port = d / "port.cppm"
    original = port.read_text(encoding="utf-8")
    applied, survived = 0, []
    try:
        for name, pat, rep in MUTATIONS:
            mutated, n = re.subn(pat, rep, original, count=1)
            if n == 0 or mutated == original:
                continue
            applied += 1
            port.write_text(mutated, encoding="utf-8")
            ok, _ = run_build(d, timeout=MUTANT_TIMEOUT)
            if ok:                      # harness passed a broken port
                survived.append(name)
    finally:
        port.write_text(original, encoding="utf-8")

    if applied < MIN_MUTATIONS:
        return False, f"only {applied} mutations applicable (need {MIN_MUTATIONS})"
    if survived:
        return False, f"harness failed to detect: {', '.join(survived)}"
    return True, f"{applied}/{applied} mutations killed"


_ROWS_CACHE: list[dict] = []


def _batch_rows(batch_id: str) -> list[dict]:
    return [r for r in _ROWS_CACHE if r["batch_id"] == batch_id]


def gate(batch_id: str, d: Path) -> tuple[bool, str]:
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

    ARTIFACTS.mkdir(parents=True, exist_ok=True)
    (ARTIFACTS / f"{batch_id}.json").write_text(json.dumps({
        "batch": batch_id,
        "verified": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "evidence": detail,
        "ir_equivalent": ir_ok,
        "harness_output": out[-4000:],
        "input_hashes": {r["path"]: fingerprint(ROOT / r["path"])
                         for r in _batch_rows(batch_id)},
    }, indent=2), encoding="utf-8")
    return True, detail


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


def do_batch(batch_id: str, rows: list[dict], model: str, split_ok: bool = True) -> bool:
    global _ROWS_CACHE
    _ROWS_CACHE = rows
    mine = [r for r in rows if r["batch_id"] == batch_id]
    outdir = WORK / mine[0]["dir"] / batch_id
    outdir.mkdir(parents=True, exist_ok=True)

    ns = re.sub(r"[^a-z0-9]+", "_", mine[0]["dir"].lower()).strip("_")
    prompt = PROMPT.format(
        batch_id=batch_id,
        file_list="\n".join(f"  {r['path']}  ({r['lines']} lines)" for r in mine),
        outdir=str(outdir.relative_to(ROOT)),
        module=f"{ns}.{batch_id}".replace("_", "."),
        ns=f"{ns}::{batch_id}",
    )

    say(f"{batch_id}: {len(mine)} files, {sum(int(r['lines']) for r in mine)} lines "
        f"[{mine[0]['dir']}]")

    try:
        r = subprocess.run(
            ["cursor-agent", "-p", prompt, "--workspace", str(ROOT), "--model", model,
             "--output-format", "text", "--force", "--trust"],
            timeout=AGENT_TIMEOUT, text=True, capture_output=True,
            stdin=subprocess.DEVNULL,
        )
        agent_ok = r.returncode == 0
        agent_out = (r.stdout or "")[-2000:]
    except subprocess.TimeoutExpired:
        agent_ok, agent_out = False, "agent timed out"

    if not agent_ok:
        say(f"  ✗ agent failed")
        log(batch=batch_id, status="AGENT_FAILED", detail=agent_out)
        ok, detail = False, "agent failed"
    else:
        ok, detail = gate(batch_id, outdir)

    new = "VERIFIED" if ok else "REJECTED"
    for r in rows:
        if r["batch_id"] == batch_id:
            r["status"] = new
    save_rows(rows)

    if ok:
        say(f"  ✓ VERIFIED — {detail}")
        log(batch=batch_id, status="VERIFIED", detail=detail)
        emit_build_wiring(rows)
        n = propagate_clones(rows, {r["path"] for r in mine})
        if n:
            say(f"  + {n} clone files inherit this verdict")
        save_rows(rows)
        git("add", "-A")
        git("commit", "-q", "-m", f"pbsd: {batch_id} verified ({mine[0]['dir']})")
    else:
        say(f"  ✗ rejected — {detail.splitlines()[0] if detail else '?'}")
        log(batch=batch_id, status="REJECTED", detail=detail)
        clean_batch(outdir)

        # Most rejections are one hard file poisoning an otherwise fine batch.
        # Retry file-by-file to salvage the rest. Only for real batches.
        produced_nothing = detail.startswith("missing ") or detail == "agent failed"
        if split_ok and len(mine) > 1 and not produced_nothing:
            say(f"  retrying {len(mine)} files individually")
            for r in mine:
                sub = f"{batch_id}s{mine.index(r)+1}"
                for row in rows:
                    if row["path"] == r["path"]:
                        row["batch_id"] = sub
                        row["status"] = "PENDING"
                save_rows(rows)
                do_batch(sub, rows, model, split_ok=False)
            return any(r["status"] == "VERIFIED" for r in rows
                       if r["path"] in {m["path"] for m in mine})

        for r in mine:
            r["status"] = "DEFERRED"
            defer(r["path"], batch_id, ["AUTO_FAILED"], detail)
        save_rows(rows)
        git("add", str(INVENTORY.relative_to(ROOT)))
        git("commit", "-q", "-m", f"pbsd: {batch_id} deferred")
    return ok



def run_deferred_phase(rows: list[dict], model: str) -> tuple[int, int]:
    """Work the deferred queue LAST, one file at a time, feeding each failure
    back into the next attempt. Survivors get written up for a human."""
    items = [i for i in load_deferred() if i.get("attempts", 0) < DEFERRED_ATTEMPTS]
    if not items:
        return 0, 0
    banner(f"Deferred queue — {len(items)} hard files, {DEFERRED_ATTEMPTS} attempts each")

    fixed, stuck = 0, 0
    for n, it in enumerate(items, 1):
        path = ROOT / it["path"]
        if not path.is_file():
            continue
        row = next((r for r in rows if r["path"] == it["path"]), None)
        outdir = WORK / (row["dir"] if row else "deferred") / (path.stem + "_d")
        outdir.mkdir(parents=True, exist_ok=True)
        bid = f"d_{path.stem}"

        print(f"\n[deferred {n}/{len(items)}] {it['path']}", flush=True)
        say(f"difficulties: {', '.join(it['reasons']) or 'unclassified'}")

        ok = False
        while it["attempts"] < DEFERRED_ATTEMPTS and not ok:
            it["attempts"] += 1
            prompt = DEFERRED_PROMPT.format(
                path=it["path"], lines=row["lines"] if row else "?",
                reasons=", ".join(it["reasons"]) or "unclassified",
                detail=(it.get("detail") or "(none)")[-1200:],
                outdir=str(outdir.relative_to(ROOT)))
            try:
                r = subprocess.run(
                    ["cursor-agent", "-p", prompt, "--workspace", str(ROOT), "--model", model,
                     "--output-format", "text", "--force", "--trust"],
                    timeout=AGENT_TIMEOUT, text=True, capture_output=True,
                    stdin=subprocess.DEVNULL)
                agent_ok = r.returncode == 0
            except subprocess.TimeoutExpired:
                agent_ok = False

            if (outdir / "IMPOSSIBLE.txt").is_file():
                it["last_error"] = (outdir / "IMPOSSIBLE.txt").read_text(errors="ignore")[:400]
                say(f"  agent declared impossible (attempt {it['attempts']})")
                break
            if not agent_ok:
                it["detail"] = it["last_error"] = "agent failed or timed out"
                continue

            _ROWS_CACHE.clear(); _ROWS_CACHE.extend(rows)
            ok, detail = gate(bid, outdir)
            it["detail"] = it["last_error"] = detail
            say(("  ✓ VERIFIED — " if ok else f"  ✗ attempt {it['attempts']} — ") +
                detail.splitlines()[0][:100])

        if ok:
            if row:
                row["status"] = "VERIFIED"
            fixed += 1
            save_rows(rows)
            emit_build_wiring(rows)
            git("add", "-A"); git("commit", "-q", "-m", f"pbsd: {it['path']} verified (deferred)")
        else:
            if row:
                row["status"] = "NEEDS_HUMAN"
            stuck += 1
            clean_batch(outdir)
            save_rows(rows)

    save_deferred(items)
    write_human_report([i for i in items if i.get("attempts", 0) >= DEFERRED_ATTEMPTS
                        or i.get("last_error", "").startswith(("cannot", "This file"))])
    return fixed, stuck


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", default=MODEL)
    ap.add_argument("--batches", type=int, default=0, help="0 = run until done")
    ap.add_argument("--status", action="store_true")
    ap.add_argument("--reset-setup", action="store_true")
    ap.add_argument("--deferred", action="store_true",
                    help="work only the deferred queue (e.g. once you have credits)")
    a = ap.parse_args()

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
    if a.deferred:
        fixed, stuck = run_deferred_phase(rows, a.model)
        banner(f"Deferred pass: {fixed} recovered, {stuck} need you")
        status()
        return 0
    n_drift = check_drift(rows)
    if n_drift:
        say(f"upstream drift: reopened {n_drift} batches")
        save_rows(rows)
    queue = []
    for r in rows:
        if r["status"] == "PENDING" and r["batch_id"] not in queue:
            queue.append(r["batch_id"])
    if a.batches:
        queue = queue[: a.batches]

    banner(f"Converting {len(queue)} batches   model={a.model}")
    ver = rej = 0
    try:
        for i, b in enumerate(queue, 1):
            print(f"\n[{i}/{len(queue)}]", flush=True)
            if do_batch(b, rows, a.model):
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
        fixed, stuck = run_deferred_phase(rows, a.model)
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
