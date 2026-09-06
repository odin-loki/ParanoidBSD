#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which functions can be model-checked SOUNDLY with no precondition.

CBMC started at an arbitrary function makes that function's parameters
nondeterministic. For a function whose parameters are all scalars that is
exactly right: every int is a possible int, and a property that holds is a
property that holds for all inputs.

For a POINTER parameter it is wrong, and the first run of this over
lib/libc/string showed how wrong:

    FAILED lib/libc/string/strcat.c:strcat
        line 41 pointer arithmetic: pointer NULL in s + 1l

strcat(3) requires a valid, NUL-terminated, non-overlapping buffer. A
nondeterministic `char *` includes NULL, includes a dangling pointer, and
includes a pointer to an unterminated array - so CBMC reports the ABSENCE
OF A PRECONDITION as a defect, on a function that has none. Every string
routine in the library fails that way, and none of those failures is real.

So the corpus splits, and the split is decidable from the signature:

  SCALAR    every parameter is an integer, float, enum or bool, and so is
            anything it returns by value. Unguarded modular checking is
            SOUND: the results mean what they say.
  POINTER   at least one pointer or array parameter. Needs a harness that
            establishes the callee's precondition before the call, and the
            result is then only as good as the precondition written. Those
            are done separately and recorded with the harness that produced
            them, because a proof under an assumption is a different claim.
  VOID      no parameters. Sound, same as SCALAR.

Types come from CBMC's own goto model rather than from parsing C, because
the goto model is what CBMC is going to check and a regex over C
declarators is not it.

Emits JSON: {file: {"ok": bool, "error": str, "functions": {name: class}}}
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
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from includes import include_flags, lang_flags  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

SYM_RE = re.compile(
    r"^Symbol\.+:\s*(?P<sym>\S+)\s*$"
    r"(?P<mid>.*?)"
    r"^Type\.+:\s*(?P<type>.*?)$",
    re.M | re.S,
)

# A parameter list is the text inside the outermost parentheses of the type.
SCALAR_WORDS = (
    "signed int", "unsigned int", "signed long", "unsigned long",
    "signed char", "unsigned char", "signed short", "unsigned short",
    "float", "double", "bool", "c_bool", "__CPROVER", "signed __int128",
    "unsigned __int128", "long double", "empty",
)


def _params(type_str: str) -> str | None:
    """The parameter list of a function type, or None if it is not one."""
    i = type_str.find("(")
    if i < 0:
        return None
    depth = 0
    for j in range(i, len(type_str)):
        if type_str[j] == "(":
            depth += 1
        elif type_str[j] == ")":
            depth -= 1
            if depth == 0:
                return type_str[i + 1:j]
    return None


PTR_GLOBAL_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


def classify_type(type_str: str) -> str:
    p = _params(type_str)
    if p is None:
        return "NOTFN"
    p = p.strip()
    if p in ("", "void", "empty"):
        return "VOID"
    if "*" in p or "[" in p:
        return "POINTER"
    # struct/union by value: CBMC handles it, but a nondeterministic struct
    # with a pointer field is the same trap one level down.
    if "struct" in p or "union" in p:
        return "POINTER"
    if any(w in p for w in SCALAR_WORDS):
        return "SCALAR"
    return "OTHER"


def model_one(job: dict) -> dict:
    src, rel, outdir = job["src"], job["rel"], job["outdir"]
    gb = Path(outdir) / (rel.replace("/", "__") + ".gb")
    gb.parent.mkdir(parents=True, exist_ok=True)
    cc = shutil.which("goto-cc") or "goto-cc"
    sp = Path(src)
    inc = include_flags(sp, job.get("arch", "amd64"))
    try:
        p = subprocess.run(
            [cc, *lang_flags(sp), *inc, "-Wno-everything", src, "-o", str(gb)],
            capture_output=True, text=True, timeout=job["timeout"],
        )
    except subprocess.TimeoutExpired:
        return {"file": rel, "ok": False, "error": "goto-cc timeout"}
    except OSError as e:
        return {"file": rel, "ok": False, "error": str(e)}
    if p.returncode != 0 or not gb.is_file():
        return {"file": rel, "ok": False,
                "error": (p.stderr or p.stdout or "").strip()[-400:]}

    gi = shutil.which("goto-instrument") or "goto-instrument"
    try:
        q = subprocess.run([gi, "--show-symbol-table", str(gb)],
                           capture_output=True, timeout=job["timeout"])
        q_out = q.stdout.decode("utf-8", "replace")
    except (subprocess.TimeoutExpired, OSError) as e:
        return {"file": rel, "ok": False, "error": f"symbol table: {e}"}

    fns: dict[str, str] = {}
    linkage: dict[str, str] = {}
    extern_ptrs: set[str] = set()
    for m in SYM_RE.finditer(q_out):
        sym, ty = m.group("sym"), m.group("type").strip()
        if "::" in sym:          # a parameter or local, not a function
            continue
        cls = classify_type(ty)
        if cls != "NOTFN":
            fns[sym] = cls
            # `static` shows in the goto symbol table as file_local, and it
            # decides how a finding must be read.
            #
            # A modular check gives a function's WHOLE signature domain. For
            # an exported function that is the right domain - it is the
            # contract callers are entitled to, so UB anywhere in it is a
            # defect. stdc_leading_ones_uc(unsigned char) is UB for every x
            # >= 128 and rint(double) for every negative x; both were real.
            #
            # For a static helper it is the wrong domain, because the only
            # callers are in the same file and they constrain it.
            # nsap_addr.c's xtob(int c) subtracts '0' or '7' and overflows
            # at INT_MIN - and its one caller passes an isxdigit() char.
            # Reporting that as a defect needs the callers, which a modular
            # check does not have.
            linkage[sym] = ("static" if "file_local" in (m.group("mid") or "")
                            else "exported")
        elif "*" in ty:
            # A file-scope POINTER defined in another translation unit.
            # CBMC has no value for it, so it is nondeterministic - which
            # means NULL, and dangling, exactly like an unconstrained
            # pointer parameter.
            extern_ptrs.add(sym)

    # Which functions touch one. getchar() takes no arguments and still
    # came back `dereference failure: pointer NULL`, because its body is
    #     CALL _flockfile(__stdinp)
    # and __stdinp lives in another unit. A function like that is no more
    # soundly checkable unguarded than one taking a char *.
    if extern_ptrs and fns:
        try:
            g = subprocess.run([gi, "--show-goto-functions", str(gb)],
                               capture_output=True, timeout=job["timeout"])
            # String literals reach this output as raw source bytes, so it
            # is not necessarily UTF-8. Only identifiers are being read.
            bodies = _split_bodies(g.stdout.decode("utf-8", "replace"))
            for name in list(fns):
                if fns[name] in ("SCALAR", "VOID"):
                    used = PTR_GLOBAL_RE.findall(bodies.get(name, ""))
                    if extern_ptrs.intersection(used):
                        fns[name] = "GLOBALPTR"
        except (subprocess.TimeoutExpired, OSError):
            pass

    return {"file": rel, "ok": True, "gb": str(gb), "functions": fns,
            "linkage": linkage}


def _split_bodies(text: str) -> dict[str, str]:
    """goto-instrument prints `name /* pretty */` then an indented body."""
    out: dict[str, str] = {}
    cur, buf = None, []
    for line in text.splitlines():
        if line and not line[0].isspace() and "/*" in line:
            if cur:
                out[cur] = "\n".join(buf)
            cur, buf = line.split(None, 1)[0], []
        elif cur is not None:
            buf.append(line)
    if cur:
        out[cur] = "\n".join(buf)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"))
    ap.add_argument("--scope", action="append", default=[])
    ap.add_argument("--outdir", default="/tmp/pbsd_goto")
    ap.add_argument("--out", default="verify_classes.json")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--limit", type=int)
    ap.add_argument("--arch", default="amd64")
    args = ap.parse_args()

    d = json.loads(Path(args.plan).read_text())
    jobs = []
    for rec in d["records"]:
        path = rec.get("path") or ""
        if args.scope and not any(path.startswith(s) for s in args.scope):
            continue
        if not rec.get("functions"):
            continue
        cand = SRC / path
        if not cand.is_file():
            cand = cand.with_suffix(".cpp")
            if not cand.is_file():
                continue
        jobs.append({"src": str(cand), "rel": path, "arch": args.arch,
                     "outdir": args.outdir, "timeout": args.timeout})
    if args.limit:
        jobs = jobs[:args.limit]

    print(f"{len(jobs)} translation units to model", flush=True)
    results: dict[str, dict] = {}
    counts: dict[str, int] = {}
    t0 = time.time()
    with ProcessPoolExecutor(max_workers=args.jobs) as ex:
        futs = [ex.submit(model_one, j) for j in jobs]
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            results[r["file"]] = r
            if r["ok"]:
                for c in r["functions"].values():
                    counts[c] = counts.get(c, 0) + 1
            else:
                counts["TU-ERROR"] = counts.get("TU-ERROR", 0) + 1
            if i % 250 == 0 or i == len(jobs):
                print(f"  [{i}/{len(jobs)}] {i/max(1e-9,time.time()-t0):.1f}/s  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)

    Path(args.out).write_text(json.dumps(results, indent=1))
    ok = sum(1 for r in results.values() if r["ok"])
    print(f"\n{ok}/{len(results)} translation units modelled")
    for k, v in sorted(counts.items()):
        print(f"  {k:9s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
