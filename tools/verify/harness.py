#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""CBMC harnesses for POINTER functions whose contract is not --null-depth.

WHY THIS FILE EXISTS

cbmc_driver.py --allow POINTER --null-depth 3 is the right tool when the
only missing precondition is "the pointer arguments are valid, non-null
objects". classify.py called that POINTER; CBMC's --min-null-tree-depth
states it; a success is recorded PROVED-ASSUMING, never PROVED.

That flag does NOT encode "NUL-terminated within the object". strcat(3)
went from 15 failures to 3 with --min-null-tree-depth 3, and the three
that survive are exactly the caller's contract: dest and src are strings,
dest has room. Reporting those as defects is the same mistake as the
original NULL-pointer run, one precondition up.

So POINTER splits again, and this split is also decidable from the
signature:

  null-depth    any POINTER whose parameters are not clearly a C string.
                cbmc_driver.py --null-depth 3. Not this file.
  string-like   name or type is char * / wchar_t * WITHOUT a size/count
                argument (strcat, strcpy, strlen, strcmp, ...). This
                file emits a small C harness that assumes bounded
                NUL-terminated buffers, starts CBMC at `harness`, and
                records PROVED-ASSUMING with harness= path and the
                assumption text. Never PROVED.

A SCALAR function has no business here. classify.py already said unguarded
checking is sound; a harness would shrink the domain and a success would
still be a different claim. emit_harness() returns None for those.

THE HARNESS, AND WHAT IT DOES NOT PROVE

Local arrays of N bytes (default 32), __CPROVER_r_ok / __CPROVER_w_ok on
them, a NUL in the last slot. For cat-shaped destinations, dest[0] = 0
as well, so strcat cannot overflow the dest object. That is a PRECONDITION,
written in the record, and a success is a proof under it - about buffers
of length <= N that are NUL-terminated, not about the POSIX signature.

--unwind defaults to N+1 so a loop that walks the buffer can close. If
the operator lowers it, BOUNDED is the honest answer, not PROVED-ASSUMING.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cbmc_driver as D  # noqa: E402
import classify  # noqa: E402
from includes import include_flags, lang_flags, USERLAND_TOP  # noqa: E402
import userland_names  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# POSIX/C string functions whose contract is a NUL-terminated buffer and
# which do not take a size. strn*, strl*, mem*, stpncpy are sized and
# belong to the null-depth pass, not this one.
BARE_STRING_STEMS = frozenset({
    "strcat", "strcpy", "stpcpy", "strlen",
    "strcmp", "strcasecmp", "strcasecmp_l",
    "strchr", "strrchr", "strchrnul",
    "strstr", "strcasestr", "strcasestr_l",
    "strpbrk", "strspn", "strcspn",
    "strtok", "strtok_r", "strdup", "strsep",
    "strcoll", "strcoll_l", "strverscmp",
    "index", "rindex",
    "wcscat", "wcscpy", "wcpcpy", "wcslen",
    "wcscmp", "wcscasecmp",
    "wcschr", "wcsrchr",
    "wcsstr", "wcspbrk",
    "wcsspn", "wcscspn",
    "wcstok", "wcsdup",
    "wcscoll", "wcscoll_l",
})

# First pointer is a writable destination that must have room for src.
# cat-shaped dests are emptied so concatenation cannot overflow N.
DEST_FIRST_STEMS = frozenset({
    "strcat", "strcpy", "stpcpy",
    "wcscat", "wcscpy", "wcpcpy",
})

# Known arities for when the goto type string is missing. "dst" is a
# writable char buffer emptied on entry; "src" is a readable string;
# "int" is an unconstrained scalar (strchr's c).
NAME_SHAPES: dict[str, tuple[str, ...]] = {
    "strcat": ("dst", "src"),
    "strcpy": ("dst", "src"),
    "stpcpy": ("dst", "src"),
    "strlen": ("src",),
    "strcmp": ("src", "src"),
    "strcasecmp": ("src", "src"),
    "strcasecmp_l": ("src", "src", "int"),
    "strchr": ("src", "int"),
    "strrchr": ("src", "int"),
    "strchrnul": ("src", "int"),
    "strstr": ("src", "src"),
    "strcasestr": ("src", "src"),
    "strcasestr_l": ("src", "src", "int"),
    "strpbrk": ("src", "src"),
    "strspn": ("src", "src"),
    "strcspn": ("src", "src"),
    "strtok": ("dst", "src"),
    "strtok_r": ("dst", "src", "char_pp"),
    "strdup": ("src",),
    "strsep": ("char_pp", "src"),
    "strcoll": ("src", "src"),
    "strcoll_l": ("src", "src", "int"),
    "strverscmp": ("src", "src"),
    "index": ("src", "int"),
    "rindex": ("src", "int"),
    "wcscat": ("wdst", "wsrc"),
    "wcscpy": ("wdst", "wsrc"),
    "wcpcpy": ("wdst", "wsrc"),
    "wcslen": ("wsrc",),
    "wcscmp": ("wsrc", "wsrc"),
    "wcscasecmp": ("wsrc", "wsrc"),
    "wcschr": ("wsrc", "int"),
    "wcsrchr": ("wsrc", "int"),
    "wcsstr": ("wsrc", "wsrc"),
    "wcspbrk": ("wsrc", "wsrc"),
    "wcsspn": ("wsrc", "wsrc"),
    "wcscspn": ("wsrc", "wsrc"),
    "wcstok": ("wdst", "wsrc", "wchar_pp"),
    "wcsdup": ("wsrc",),
    "wcscoll": ("wsrc", "wsrc"),
    "wcscoll_l": ("wsrc", "wsrc", "int"),
}

LENGTH_NAMES = frozenset({
    "n", "len", "nitems", "size", "nbytes", "count", "maxlen",
    "buflen", "l", "nbyte", "num", "maxsize", "maxlenp",
})

CHAR_PTR_RE = re.compile(
    r"(?:const\s+)?(?:unsigned\s+)?(?:signed\s+)?char(?:\s+const)?\s*\*"
)
WCHAR_PTR_RE = re.compile(r"(?:const\s+)?wchar_t(?:\s+const)?\s*\*")
PP_RE = re.compile(r"\*\s*\*")
SIZE_TY_RE = re.compile(r"\b(?:size_t|rsize_t)\b")
# strn*/strl*/mem* take a count. `strl` is a PREFIX of `strlen`, so this
# cannot be a startswith("strl") without excluding strlen/wcslen.


def stem_of(name: str) -> str:
    n = name.split(".")[-1]
    n = n.lstrip("_")
    if n.endswith("_l") and n[: -2] in BARE_STRING_STEMS:
        return n
    return n


def _is_sized_name(stem: str) -> bool:
    if stem.startswith(("strn", "wcsn", "mem")):
        return True
    # strlcat/strlcpy, not strlen
    if stem.startswith("strl") and stem != "strlen":
        return True
    if stem.startswith("wcsl") and stem != "wcslen":
        return True
    return stem in {"stpncpy", "wcpncpy", "strxfrm", "strxfrm_l",
                    "wcsxfrm", "wcsxfrm_l", "strnstr"}


def _split_comma(params: str) -> list[str]:
    parts, buf, depth = [], [], 0
    for ch in params:
        if ch == "(":
            depth += 1
            buf.append(ch)
        elif ch == ")":
            depth -= 1
            buf.append(ch)
        elif ch == "," and depth == 0:
            parts.append("".join(buf).strip())
            buf = []
        else:
            buf.append(ch)
    tail = "".join(buf).strip()
    if tail:
        parts.append(tail)
    return parts


def param_kind(decl: str) -> str:
    s = decl.strip()
    if SIZE_TY_RE.search(s):
        return "size"
    stars = s.count("*")
    if WCHAR_PTR_RE.search(s):
        return "wchar_pp" if stars >= 2 or PP_RE.search(s) else "wchar_ptr"
    if CHAR_PTR_RE.search(s):
        return "char_pp" if stars >= 2 or PP_RE.search(s) else "char_ptr"
    if "*" in s or "[" in s:
        return "ptr"
    if re.search(r"\b(?:int|long|short|unsigned|signed|bool|_Bool|char)\b", s):
        return "int"
    return "other"


def parse_params(type_str: str) -> list[dict] | None:
    raw = classify._params(type_str)
    if raw is None:
        return None
    raw = raw.strip()
    if raw in ("", "void", "empty"):
        return []
    return [{"raw": p, "kind": param_kind(p)} for p in _split_comma(raw)]


def _ident(decl: str) -> str:
    m = re.search(r"([A-Za-z_][A-Za-z0-9_]*)$", decl.strip())
    return m.group(1) if m else ""


def trailing_length(params: list[dict]) -> bool:
    if not params:
        return False
    last = params[-1]
    if last["kind"] == "size":
        return True
    if last["kind"] != "int":
        return False
    if _ident(last["raw"]) in LENGTH_NAMES:
        return True
    ptrs = sum(1 for p in params if "ptr" in p["kind"] or p["kind"] == "ptr")
    if ptrs >= 2 and re.search(r"\b(?:unsigned|long|size_t)\b", last["raw"]):
        return True
    return False


def _stem_in_bare(stem: str) -> bool:
    if stem in BARE_STRING_STEMS:
        return True
    for s in BARE_STRING_STEMS:
        if stem.endswith(s):
            return True
    return False


def is_string_like(name: str, type_str: str | None = None) -> bool:
    """char * / wchar_t * without a size. SCALAR is never this.

    Type, when present, wins: a size_t / trailing length makes this False
    even if the name looks like strcat. A name in BARE_STRING_STEMS is
    enough when there is no type. A type of only char*/wchar_t* (and
    optional int, as strchr) is enough when the name is unknown.
    """
    stem = stem_of(name)
    if _is_sized_name(stem):
        return False
    params = parse_params(type_str) if type_str else None
    if params:
        kinds = [p["kind"] for p in params]
        if "size" in kinds or trailing_length(params):
            return False
        if any(k in ("ptr", "other") for k in kinds):
            # void *, struct *, ... - not a C string contract.
            return False
        if any(k in ("char_ptr", "wchar_ptr", "char_pp", "wchar_pp")
               for k in kinds):
            return True
        return False
    return _stem_in_bare(stem)


def _dest_first(stem: str) -> bool:
    if stem in DEST_FIRST_STEMS:
        return True
    return any(stem.endswith(s) for s in DEST_FIRST_STEMS)


def _shape_from_params(stem: str, params: list[dict]) -> tuple[str, ...] | None:
    dest = _dest_first(stem)
    used_dest = False
    out: list[str] = []
    for p in params:
        k = p["kind"]
        if k == "char_ptr":
            if dest and not used_dest:
                out.append("dst")
                used_dest = True
            else:
                out.append("src")
        elif k == "wchar_ptr":
            if dest and not used_dest:
                out.append("wdst")
                used_dest = True
            else:
                out.append("wsrc")
        elif k == "char_pp":
            out.append("char_pp")
        elif k == "wchar_pp":
            out.append("wchar_pp")
        elif k == "int":
            out.append("int")
        else:
            return None
    if not any(x in ("dst", "src", "wdst", "wsrc", "char_pp", "wchar_pp")
               for x in out):
        return None
    return tuple(out)


def shape_for(name: str, type_str: str | None = None) -> tuple[str, ...] | None:
    """Parameter roles we can synthesise, or None if we should not emit."""
    if not is_string_like(name, type_str):
        return None
    stem = stem_of(name)
    params = parse_params(type_str) if type_str else None
    if params:
        return _shape_from_params(stem, params)
    if stem in NAME_SHAPES:
        return NAME_SHAPES[stem]
    for s, shape in NAME_SHAPES.items():
        if stem.endswith(s):
            return shape
    return None


def assumption_text(name: str, shape: tuple[str, ...], n: int) -> str:
    bits = [
        f"bounded NUL-terminated buffers of {n} bytes/wchars",
        "__CPROVER_r_ok / __CPROVER_w_ok on those objects",
    ]
    if any(s in ("dst", "wdst") for s in shape):
        bits.append("destination initially empty so the result fits")
    bits.append(f"callee {name}; not a proof over the signature domain")
    return "; ".join(bits)


def emit_harness(name: str, type_str: str | None = None, *,
                 orig_src: Path | None = None, bound: int = 32) -> str | None:
    """A C translation unit that calls `name` under the string assumption.

    Returns None for scalar-shaped signatures and for POINTER signatures
    this file will not pretend to know the precondition of.
    """
    shape = shape_for(name, type_str)
    if not shape:
        return None
    lines: list[str] = [
        "/* Generated by tools/verify/harness.py.",
        " * A success is PROVED-ASSUMING, never PROVED: the assumptions",
        " * below are the claim, not the POSIX/C signature.",
        " */",
        f"#ifndef PBSD_HARNESS_N",
        f"#define PBSD_HARNESS_N {int(bound)}",
        f"#endif",
    ]
    if orig_src is not None:
        # Absolute include, so goto-cc's working directory does not matter.
        inc = orig_src.resolve().as_posix()
        lines.append(f'#include "{inc}"')
    lines.append("")
    lines.append("void harness(void)")
    lines.append("{")
    args: list[str] = []
    n_int = 0
    for i, role in enumerate(shape):
        if role == "src":
            lines += [
                f"    char p{i}[PBSD_HARNESS_N];",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}, sizeof(p{i})));",
                f"    p{i}[sizeof(p{i}) - 1] = '\\0';",
            ]
            args.append(f"p{i}")
        elif role == "dst":
            lines += [
                f"    char p{i}[PBSD_HARNESS_N];",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}, sizeof(p{i})));",
                f"    __CPROVER_assume(__CPROVER_w_ok(p{i}, sizeof(p{i})));",
                f"    p{i}[sizeof(p{i}) - 1] = '\\0';",
                f"    p{i}[0] = '\\0';",
            ]
            args.append(f"p{i}")
        elif role == "wsrc":
            lines += [
                f"    wchar_t p{i}[PBSD_HARNESS_N];",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}, sizeof(p{i})));",
                f"    p{i}[sizeof(p{i}) / sizeof(p{i}[0]) - 1] = L'\\0';",
            ]
            args.append(f"p{i}")
        elif role == "wdst":
            lines += [
                f"    wchar_t p{i}[PBSD_HARNESS_N];",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}, sizeof(p{i})));",
                f"    __CPROVER_assume(__CPROVER_w_ok(p{i}, sizeof(p{i})));",
                f"    p{i}[sizeof(p{i}) / sizeof(p{i}[0]) - 1] = L'\\0';",
                f"    p{i}[0] = L'\\0';",
            ]
            args.append(f"p{i}")
        elif role == "char_pp":
            lines += [
                f"    char p{i}_buf[PBSD_HARNESS_N];",
                f"    char *p{i} = p{i}_buf;",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}_buf, sizeof(p{i}_buf)));",
                f"    __CPROVER_assume(__CPROVER_w_ok(p{i}_buf, sizeof(p{i}_buf)));",
                f"    p{i}_buf[sizeof(p{i}_buf) - 1] = '\\0';",
            ]
            args.append(f"&p{i}")
        elif role == "wchar_pp":
            lines += [
                f"    wchar_t p{i}_buf[PBSD_HARNESS_N];",
                f"    wchar_t *p{i} = p{i}_buf;",
                f"    __CPROVER_assume(__CPROVER_r_ok(p{i}_buf, sizeof(p{i}_buf)));",
                f"    __CPROVER_assume(__CPROVER_w_ok(p{i}_buf, sizeof(p{i}_buf)));",
                f"    p{i}_buf[sizeof(p{i}_buf) / sizeof(p{i}_buf[0]) - 1] = L'\\0';",
            ]
            args.append(f"&p{i}")
        elif role == "int":
            lines.append(f"    int i{n_int};")
            args.append(f"i{n_int}")
            n_int += 1
        else:
            return None
    lines.append(f"    (void){name}({', '.join(args)});")
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


_SRC_DECL_RE_CACHE: dict[str, re.Pattern[str]] = {}


def type_from_source(text: str, fn: str) -> str | None:
    """Best-effort C declarator. Wrong parses are dropped, not guessed at."""
    if fn not in _SRC_DECL_RE_CACHE:
        _SRC_DECL_RE_CACHE[fn] = re.compile(
            r"\b" + re.escape(fn) + r"\s*\(", re.M)
    for m in _SRC_DECL_RE_CACHE[fn].finditer(text):
        line_start = text.rfind("\n", 0, m.start()) + 1
        prefix = text[line_start:m.start()]
        if "#define" in prefix or "#undef" in prefix:
            continue
        params = classify._params("x" + text[m.end() - 1: m.end() + 800])
        if params is None:
            continue
        pret = prefix.strip().rstrip("*").strip()
        if not pret:
            prev = text.rfind("\n", 0, line_start - 1 if line_start else 0)
            pret = text[prev + 1:line_start].strip()
        if not pret or pret.startswith("#") or pret.endswith("\\"):
            pret = "int"
        return f"{pret} ({params})"
    return None


def _compile_harness(harness_c: Path, orig: Path, rel: str,
                     out_gb: Path, arch: str, timeout: int) -> str | None:
    """goto-cc the harness (which #includes the original). None on success."""
    cc = shutil.which("goto-cc") or "goto-cc"
    try:
        inc = include_flags(orig, arch, cc="goto-cc")
        lang = lang_flags(orig, rel, as_c=True)
    except Exception as e:
        return f"include flags: {e}"
    extra_i = [f"-I{orig.parent}"]
    cmd = [cc, *lang, *inc, *extra_i, "-Wno-everything",
           str(harness_c), "-o", str(out_gb)]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return "goto-cc timeout"
    except OSError as e:
        return str(e)
    if p.returncode != 0 or not out_gb.is_file():
        return (p.stderr or p.stdout or "").strip()[-400:] or "goto-cc failed"
    return None


def verify_harness_one(task: dict) -> dict:
    """Generate, compile, CBMC --function harness. Status never PROVED."""
    fn = task["function"]
    rel = task["file"]
    bound = task.get("bound", 32)
    orig = Path(task["orig_src"])
    text = emit_harness(fn, task.get("type_str"), orig_src=orig, bound=bound)
    if text is None:
        return D._rec(task, "ERROR",
                      detail="no string harness for this signature")
    hdir = Path(task["harness_dir"])
    hdir.mkdir(parents=True, exist_ok=True)
    safe = (rel.replace("/", "__") + "__" + fn).replace(" ", "_")
    harness_c = hdir / (safe + ".c")
    harness_c.write_text(text, encoding="utf-8")
    gb = hdir / (safe + ".gb")
    t0 = time.time()
    err = _compile_harness(harness_c, orig, rel, gb,
                           task.get("arch", "amd64"), task["timeout"])
    if err:
        work = dict(task)
        work["harness"] = str(harness_c)
        work["assuming"] = assumption_text(
            fn, shape_for(fn, task.get("type_str")) or ("src",), bound)
        work["orig_function"] = fn
        return D._rec(work, "ERROR", detail=err, elapsed=time.time() - t0)

    work = dict(task)
    work["src"] = str(gb)
    work["function"] = "harness"
    work["orig_function"] = fn
    work["harness"] = str(harness_c)
    work["assuming"] = assumption_text(
        fn, shape_for(fn, task.get("type_str")) or ("src",), bound)
    work["class"] = "POINTER"
    work["null_depth"] = 0
    remaining = max(1, task["timeout"] - int(time.time() - t0))
    work["timeout"] = remaining
    return D.verify_one(work)


def _priority(path: str) -> tuple[int, str]:
    if path.startswith("lib/libc/string"):
        return (0, path)
    if path.startswith("sys/kern"):
        return (1, path)
    return (2, path)


def load_string_tasks(plan: Path, classes: Path, scopes: list[str],
                      unwind: int, timeout: int, bound: int,
                      harness_dir: Path, arch: str, src_root: Path,
                      mem_mb: int = 0) -> list[dict]:
    """POINTER ∩ ledger ∩ string-like, libc string first then sys/kern."""
    tasks = D.load_tasks(
        plan, scopes, unwind, timeout, 0, "ub", classes, {"POINTER"},
        null_depth=0, mem_mb=mem_mb)
    skipped = {"not-string": 0, "no-source": 0, "no-shape": 0, "no-type-file": 0}
    src_cache: dict[str, str] = {}
    out: list[dict] = []
    for t in tasks:
        rel = t["file"]
        orig = src_root / rel
        if not orig.is_file():
            skipped["no-source"] += 1
            continue
        if rel not in src_cache:
            try:
                src_cache[rel] = orig.read_text(encoding="utf-8",
                                                errors="replace")
            except OSError:
                skipped["no-type-file"] += 1
                continue
        type_str = type_from_source(src_cache[rel], t["function"])
        if not is_string_like(t["function"], type_str):
            skipped["not-string"] += 1
            continue
        if shape_for(t["function"], type_str) is None:
            skipped["no-shape"] += 1
            continue
        t = dict(t)
        t["type_str"] = type_str
        t["orig_src"] = str(orig)
        t["bound"] = bound
        t["harness_dir"] = str(harness_dir)
        t["arch"] = arch
        t["null_depth"] = 0
        t["class"] = "POINTER"
        out.append(t)
    out.sort(key=lambda t: (*_priority(t["file"]), t["function"]))
    print("  string-harness skipped: "
          + "  ".join(f"{k}={v}" for k, v in skipped.items()), flush=True)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"))
    ap.add_argument("--scope", action="append", default=[],
                    help="restrict to paths starting with this (repeatable). "
                         "First cut: lib/libc/string and sys/kern")
    ap.add_argument("--unwind", type=int, default=0,
                    help="CBMC --unwind. 0 (default) is bound+1, so a loop "
                         "walking the assumed buffer can close")
    ap.add_argument("--bound", type=int, default=32,
                    help="NUL-terminated buffer size in the harness")
    ap.add_argument("--timeout", type=int, default=60)
    ap.add_argument("--mem-mb", type=int, default=0)
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--classes", default="verify_classes.json")
    ap.add_argument("--harness-dir", default="/tmp/pbsd_harness",
                    help="generated .c and .gb files land here")
    ap.add_argument("--arch", default="amd64")
    ap.add_argument("--out", default="verify_harness.jsonl")
    ap.add_argument("--limit", type=int,
                    help="cap the first cut so this cannot run for days")
    ap.add_argument("--resume", action="store_true")
    args = ap.parse_args()

    unwind = args.unwind if args.unwind > 0 else args.bound + 1
    src_root = SRC
    tasks = load_string_tasks(
        Path(args.plan), Path(args.classes), args.scope or ["sys", "lib"],
        unwind, args.timeout, args.bound, Path(args.harness_dir),
        args.arch, src_root, args.mem_mb)

    out = Path(args.out)
    done: set[tuple[str, str]] = set()
    if not args.resume and out.is_file():
        out.write_text("")
    if args.resume and out.is_file():
        keep = []
        stale = 0
        for line in out.read_text().splitlines():
            if not line.strip():
                continue
            try:
                r = json.loads(line)
            except ValueError:
                continue
            if r.get("v") != D.RESULT_VERSION:
                stale += 1
                continue
            try:
                done.add((r["file"], r["function"]))
            except KeyError:
                continue
            keep.append(line)
        if stale:
            out.write_text("".join(l + "\n" for l in keep))
        tasks = [t for t in tasks if (t["file"], t["function"]) not in done]

    if args.limit is not None:
        tasks = tasks[: args.limit]

    print(f"{len(tasks)} string-like POINTER functions to harness"
          + (f", {len(done)} already done" if done else ""), flush=True)
    if not tasks:
        return 0

    if any(t["file"].split("/")[0] in USERLAND_TOP for t in tasks):
        userland_names.warm((args.arch,))

    counts: dict[str, int] = {}
    t0 = time.time()
    with out.open("a") as fh, ProcessPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(verify_harness_one, t): t for t in tasks}
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            counts[r["status"]] = counts.get(r["status"], 0) + 1
            fh.write(json.dumps(r) + "\n")
            fh.flush()
            if r["status"] == "FAILED":
                print(f"  FAILED {r['file']}:{r['function']}", flush=True)
            if i % 50 == 0 or i == len(tasks):
                rate = i / max(1e-9, time.time() - t0)
                print(f"  [{i}/{len(tasks)}] {rate:.1f}/s  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)

    print("\n== totals")
    for k, v in sorted(counts.items()):
        print(f"  {k:9s} {v}")
    proved = counts.get("PROVED", 0)
    if proved:
        print(f"\nFAIL  {proved} record(s) said PROVED under a harness. "
              "That is a false theorem; this driver must not emit it.",
              flush=True)
        return 1
    checked = sum(v for k, v in counts.items() if k != "ERROR")
    if tasks and checked == 0:
        print(f"\nFAIL  {len(tasks)} function(s) and not one could be checked",
              flush=True)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
