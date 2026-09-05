#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate the port ledger: every file PBSD owns, and every function in it.

docs/ROADMAP.md says what to do at the level of subsystems. This is the
other end of the telescope - one numbered line per file and per function,
tagged, so the work can be picked up an item at a time by somebody who was
not here for the argument.

It is generated, and it says so, because sixteen thousand files cannot be
kept accurate by hand and a stale ledger is worse than none. Re-run it and
commit the diff.

NUMBERING

  A       area          1..N, fixed order, see AREAS
  A.D     directory     within the area, sorted by path
  A.D.F   file          within the directory, sorted by name
  A.D.F.N function      within the file, in the order they appear

The numbers are positional, so inserting a file renumbers what follows. That
is deliberate: the ledger is regenerated, not edited, and a tag plus a path
is what identifies an item. Do not cite a number in a commit message; cite
the path.

TAGS

  TODO-PORT     C that PBSD intends to port to C++23. The default.
  TODO-ASM      assembly. Either it becomes C++ or it is one of the cases
                docs/ASSEMBLY.md calls irreducible - trap vectors, context
                switch, early boot - and the tag becomes TODO-KEEP.
  TODO-MD       machine-dependent, under sys/<arch> or lib/<arch>. Porting
                it means deciding whether it should exist at all; see
                docs/CONSOLIDATION.md.
  TODO-HDR      a header. Ports follow their consumers rather than leading.
  TODO-GEN      generated, or generatable. Porting the output is the wrong
                end - see the linux_systrace_args.c note in
                docs/CONSOLIDATION.md.
  TODO-DATA     a .c file that is an array of numbers rather than a
                program - firmware blobs, register tables, init values.
                sys/dev/qlxgbe/ql_fw.c is 149,069 lines and defines no
                function.
  TODO-DECIDE   a subsystem whose presence is a product decision before it
                is a porting one: setuid programs, sendmail, Kerberos.
  SKIP-VENDOR   contrib/, crypto/, cddl/. Not ours, and a local edit is a
                merge conflict for ever.
  SKIP-TESTS    tests/. Runs against the port; is not part of it.
  DONE-MODULE   a C++23 module for this already exists under pbsd/.

WHAT THE FUNCTION LIST IS AND IS NOT

Function names come from a regex for a definition starting at column zero,
which is the style this tree uses. It will miss anything produced by a macro
- and this tree does that a lot; arm64's atomic.h generates its whole
interface and no regex will ever see it - and it will occasionally catch a
declaration. The count is therefore a floor, and the module docstring of
tools/atomic_survey.py explains why that matters here more than it would
elsewhere.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "hbsd/src"

# (number, path prefix, title, default tag)
AREAS = [
    ("lib", "lib", "Userland libraries", "TODO-PORT"),
    ("libexec", "libexec", "Userland services and the dynamic linker", "TODO-PORT"),
    ("bin", "bin", "Essential user commands", "TODO-PORT"),
    ("sbin", "sbin", "Essential system commands", "TODO-PORT"),
    ("usr.bin", "usr.bin", "User commands", "TODO-PORT"),
    ("usr.sbin", "usr.sbin", "System commands", "TODO-PORT"),
    ("stand", "stand", "Boot loader", "TODO-PORT"),
    ("sys", "sys", "Kernel", "TODO-PORT"),
    ("secure", "secure", "OpenSSH and TLS glue", "TODO-DECIDE"),
    ("kerberos5", "kerberos5", "Kerberos build glue", "TODO-DECIDE"),
    ("krb5", "krb5", "Kerberos build glue", "TODO-DECIDE"),
    ("gnu", "gnu", "GPL-licensed remnants", "TODO-DECIDE"),
    ("rescue", "rescue", "Static rescue image", "TODO-PORT"),
    ("share", "share", "Makefiles, manuals, examples", "TODO-PORT"),
    ("release", "release", "Release engineering", "TODO-PORT"),
    ("etc", "etc", "Default configuration", "TODO-DECIDE"),
    ("targets", "targets", "DIRDEPS build description", "TODO-PORT"),
    ("include", "include", "System headers", "TODO-HDR"),
    ("tools", "tools", "Build and test tooling", "TODO-PORT"),
    ("cddl", "cddl", "ZFS and DTrace (CDDL)", "SKIP-VENDOR"),
    ("crypto", "crypto", "OpenSSL and friends", "SKIP-VENDOR"),
    ("contrib", "contrib", "Third-party source", "SKIP-VENDOR"),
    ("tests", "tests", "The test suite", "SKIP-TESTS"),
]

SOURCE = (".c", ".h", ".S", ".cc", ".cpp", ".cxx", ".hh", ".hpp", ".cppm")

ARCHDIR = re.compile(
    r"(?:^|/)(amd64|arm64|aarch64|arm|armv[67]|i386|i387|powerpc|powerpc64|"
    r"powerpcspe|riscv|riscv64|x86|mips|sparc64)(?:/|$)")

# A definition whose name starts at column zero: the style this tree uses.
FUNC = re.compile(
    r"^([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*?\)\s*(?:__[a-z_]+\s*)*\{", re.M)
# ... and the one-line form that some short files use.
FUNC1 = re.compile(
    r"^(?:static\s+)?(?:inline\s+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?"
    r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{]*\)\s*\{", re.M)
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
GENERATOR = re.compile(r"^[A-Z_][A-Z0-9_]{3,}\s*\(", re.M)

GENERATED_HINT = re.compile(
    r"generated|DO NOT EDIT|automatically|created by|\bmakesyscalls\b", re.I)

# Third-party source that does not live under contrib/. sys/crypto/openssl is
# OpenSSL's perlasm output and sys/cddl is Sun's; editing either is a merge
# conflict for ever, the same as contrib/.
VENDOR_PATHS = (
    "contrib/", "crypto/", "cddl/", "gnu/",
    "sys/contrib/", "sys/crypto/openssl", "sys/cddl/", "sys/gnu/",
    "sys/dev/isp/asm_", "sys/teken/libteken",
)

# A line that is data rather than code.
#
# Two false starts. The first wanted one number per line and matched nothing,
# because a firmware array puts twelve to a line:
#
#   0x00, 0x00, 0x00, 0x00, 0x7e, 0x8f, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00,
#
# The second was a regex with a nested quantifier for exactly that shape:
#
#   ^[\s{},]*(?:(?:0[xX][0-9a-fA-F]+|[-+]?\d+)[\s{},]*)+$
#
# which backtracks catastrophically on the lines that do NOT match - and in a
# 149,000-line file that is most of what it is asked about. It pinned a core
# for four minutes on one file.
#
# So no regex. A data line is one whose characters all come from the alphabet
# of numbers and separators and which contains at least one digit. Linear,
# and it cannot be tricked.
_DATA_CHARS = frozenset("0123456789abcdefABCDEFxX \t{},-+")
_DIGITS = frozenset("0123456789")


def _is_data_line(line: str) -> bool:
    if not line or len(line) > 400:
        return False
    if not _DIGITS.intersection(line):
        return False
    return not (set(line) - _DATA_CHARS)


def looks_like_data(text: str, nfuncs: int, lines: int) -> bool:
    """A .c file that is an array of numbers, not a program.

    sys/dev/qlxgbe/ql_fw.c is 149,069 lines and defines no function: it is
    firmware, shipped as a C array. Listing it as the largest porting job in
    the tree is true by line count and useless as a plan, so it gets its own
    tag.
    """
    if nfuncs or lines < 500:
        return False
    sample = text.splitlines()[:4000]
    if not sample:
        return False
    data = sum(1 for l in sample if _is_data_line(l))
    return data > len(sample) * 0.5


def classify(rel: str, default: str, text: str | None) -> str:
    if any(rel.startswith(v) for v in VENDOR_PATHS) or "/contrib/" in rel:
        return "SKIP-VENDOR"
    if rel.startswith("tests/") or "/tests/" in rel:
        return "SKIP-TESTS"
    if rel.endswith(".S"):
        return "TODO-ASM"
    if text and GENERATED_HINT.search(text[:2000]):
        return "TODO-GEN"
    if ARCHDIR.search(rel):
        return "TODO-MD"
    if rel.endswith((".h", ".hh", ".hpp")):
        return "TODO-HDR"
    return default


def functions(text: str) -> list[str]:
    body = COMMENT.sub("", text)
    names: list[str] = []
    seen: set[str] = set()
    for rx in (FUNC, FUNC1):
        for m in rx.finditer(body):
            n = m.group(1)
            if n in seen:
                continue
            if n in ("if", "for", "while", "switch", "return", "sizeof",
                     "do", "else", "typedef", "struct", "union", "enum"):
                continue
            seen.add(n)
            names.append(n)
    return names


def modules_by_dir() -> dict[str, int]:
    """Source directory -> number of C++23 modules under the matching
    pbsd/ directory.

    By path, not by basename. The first version of this matched module stems
    against source stems and reported 150 of bin's 162 files as already
    ported, because pbsd/userland carries thousands of modules named after
    the symbol they wrap - accept, abort, a64l - and `cat` is a name that
    occurs more than once in a tree this size.

    A direct path match claims less and is true. Where pbsd/ has no directory
    of the same name, nothing is claimed: the flat-filename scheme under
    pbsd/userland encodes the path in the filename and mapping it back is an
    open problem, not something to guess at in a ledger.
    """
    out: dict[str, int] = {}
    pbsd = ROOT / "pbsd"
    if not pbsd.is_dir():
        return out
    for p in pbsd.rglob("*.cppm"):
        d = p.parent.relative_to(pbsd)
        # pbsd/bin/cat/b0163/port.cppm -> bin/cat, and also bin/cat/b0163
        parts = list(d.parts)
        while parts:
            out[Path(*parts).as_posix()] = out.get(
                Path(*parts).as_posix(), 0) + 1
            parts.pop()
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="docs/PORT_PLAN.md")
    ap.add_argument("--json", default="docs/port_plan.json")
    ap.add_argument("--max-func-lines", type=int, default=4000,
                    help="do not list functions for files larger than this; "
                         "the port unit there is the file, not the function")
    ap.add_argument("--areas", default="",
                    help="comma-separated area names, for a partial run")
    args = ap.parse_args()

    want = set(a.strip() for a in args.areas.split(",") if a.strip())
    modules = modules_by_dir()

    out_path = Path(args.out)
    if not out_path.is_absolute():
        out_path = ROOT / out_path
    json_path = Path(args.json)
    if not json_path.is_absolute():
        json_path = ROOT / json_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fh = out_path.open("w", encoding="utf-8")

    totals: dict[str, int] = {}
    area_rows = []
    records = []

    def w(line: str = "") -> None:
        fh.write(line + "\n")

    # ---- header -----------------------------------------------------------
    w("# PBSD port ledger")
    w()
    w("**Generated by `tools/port_plan.py`. Do not edit; re-run and commit "
      "the diff.**")
    w()
    w("One numbered entry per file PBSD owns, and per function in it. "
      "`docs/ROADMAP.md`")
    w("is the same work at the level of subsystems and is the one to read "
      "first; this")
    w("is the ledger you pick an item out of.")
    w()
    w("## How to read it")
    w()
    w("```")
    w("A       area       1..N, fixed order")
    w("A.D     directory  within the area, sorted by path")
    w("A.D.F   file       within the directory, sorted by name")
    w("A.D.F.N function   within the file, in source order")
    w("```")
    w()
    w("Numbers are positional. Inserting a file renumbers what follows, "
      "which is")
    w("fine because the ledger is regenerated rather than edited — **cite "
      "the path in")
    w("a commit message, never the number.**")
    w()
    w("| tag | meaning |")
    w("|---|---|")
    w("| `TODO-PORT` | C that PBSD intends to port to C++23. The default. |")
    w("| `TODO-ASM` | assembly: rewrite, drop, or reclassify `TODO-KEEP` "
      "per `docs/ASSEMBLY.md`. |")
    w("| `TODO-MD` | machine-dependent. Porting it starts with whether it "
      "should exist — `docs/CONSOLIDATION.md`. |")
    w("| `TODO-HDR` | a header. Follows its consumers rather than leading "
      "them. |")
    w("| `TODO-GEN` | generated. Porting the output is the wrong end. |")
    w("| `TODO-DATA` | a C file that is an array of numbers — firmware, "
      "tables, init values. Not a port. |")
    w("| `TODO-DECIDE` | a product decision before a porting one. |")
    w("| `SKIP-VENDOR` | `contrib/`, `crypto/`, `cddl/`. A local edit is a "
      "merge conflict for ever. |")
    w("| `SKIP-TESTS` | `tests/`. Runs against the port; is not part of "
      "it. |")
    w("| `DONE-MODULE` | unused — see the note on `pbsd/` below. |")
    w()
    w("**What is already ported is recorded per directory, not per file.** A "
      "heading")
    w("that says *N module(s) under `pbsd/lib/libc/stdio`* means the "
      "directory has")
    w("that many `.cppm` files; it does not say which of the sources below "
      "they")
    w("correspond to. The first version of this matched module names against "
      "source")
    w("names and reported 150 of `bin`'s 162 files as ported, because "
      "`pbsd/userland`")
    w("carries thousands of modules named after the symbol they wrap and "
      "`cat` is not")
    w("a unique name in a tree this size. Mapping `pbsd/userland`'s flat "
      "filenames")
    w("back to source paths is `docs/ROADMAP.md` §5, and is not something to "
      "guess")
    w("at in a ledger.")
    w()
    w("**The function list is a floor, not an inventory.** Names come from a "
      "regex for")
    w("a definition starting at column zero. Anything a macro generates is "
      "invisible to")
    w("it — arm64's `atomic.h` produces its entire interface that way and no "
      "regex will")
    w("ever see a single name. Where a file's `gen` count is high, read the "
      "file.")
    w()

    # ---- body -------------------------------------------------------------
    a_no = 0
    for key, prefix, title, default_tag in AREAS:
        if want and key not in want:
            continue
        base = SRC / prefix
        if not base.is_dir():
            continue
        a_no += 1

        bydir: dict[str, list[Path]] = {}
        for p in base.rglob("*"):
            if not p.is_file() or p.suffix not in SOURCE:
                continue
            d = p.parent.relative_to(SRC).as_posix()
            bydir.setdefault(d, []).append(p)

        a_files = sum(len(v) for v in bydir.values())
        a_funcs = 0
        a_lines = 0
        a_tags: dict[str, int] = {}

        w(f"## {a_no}. `{prefix}` — {title}")
        w()
        w(f"{a_files:,} source files in {len(bydir):,} directories.")
        w()

        # An area PBSD does not own gets one line per directory instead of
        # one per file. contrib alone is 25,000 files and a ledger entry for
        # each would be 25,000 lines saying "do not touch this", which is
        # not a plan, it is ballast. The counts are still here so the area is
        # accounted for rather than absent.
        summarise = default_tag.startswith("SKIP")
        if summarise:
            w(f"*Not ours. One line per directory; see the `{default_tag}` "
              f"row in the tag table.*")
            w()
            for d_no, d in enumerate(sorted(bydir), start=1):
                files = bydir[d]
                nl = 0
                for p in files:
                    try:
                        nl += p.read_text(errors="replace").count("\n")
                    except OSError:
                        pass
                a_lines += nl
                a_tags[default_tag] = a_tags.get(default_tag, 0) + len(files)
                totals[default_tag] = totals.get(default_tag, 0) + len(files)
                w(f"- [ ] **{a_no}.{d_no}** `{d}` · {len(files)} files, "
                  f"{nl:,}L · `{default_tag}`")
                records.append({"n": f"{a_no}.{d_no}", "path": d,
                                "lines": nl, "tag": default_tag,
                                "files": len(files)})
            w()
            area_rows.append((a_no, prefix, a_files, a_lines, 0,
                              dict(sorted(a_tags.items()))))
            w()
            continue

        for d_no, d in enumerate(sorted(bydir), start=1):
            files = sorted(bydir[d], key=lambda p: p.name)
            nmod = modules.get(d, 0)
            note = (f" · {nmod} module(s) under `pbsd/{d}`"
                    if nmod else "")
            w(f"### {a_no}.{d_no} `{d}`{note}")
            w()
            for f_no, p in enumerate(files, start=1):
                rel = p.relative_to(SRC).as_posix()
                try:
                    text = p.read_text(errors="replace")
                except OSError:
                    text = ""
                lines = text.count("\n") + 1 if text else 0
                a_lines += lines
                tag = classify(rel, default_tag, text)
                a_tags[tag] = a_tags.get(tag, 0) + 1
                totals[tag] = totals.get(tag, 0) + 1

                gen = len(GENERATOR.findall(COMMENT.sub("", text))) \
                    if text else 0
                asm = "asm " if (p.suffix == ".S" or "__asm" in text) else ""
                flags = f"{lines:,}L {asm}"
                if gen:
                    flags += f"gen:{gen} "

                fns: list[str] = []
                if (p.suffix in (".c", ".cc", ".cpp", ".cxx")
                        and lines <= args.max_func_lines and tag not in
                        ("SKIP-VENDOR", "SKIP-TESTS")):
                    fns = functions(text)
                a_funcs += len(fns)
                if (tag == "TODO-PORT" and p.suffix == ".c"
                        and looks_like_data(text, len(fns), lines)):
                    tag = "TODO-DATA"
                    a_tags["TODO-PORT"] -= 1
                    totals["TODO-PORT"] -= 1
                    a_tags[tag] = a_tags.get(tag, 0) + 1
                    totals[tag] = totals.get(tag, 0) + 1

                num = f"{a_no}.{d_no}.{f_no}"
                w(f"- [ ] **{num}** `{p.name}` · {flags.strip()} · "
                  f"`{tag}`" + (f" · {len(fns)} fn" if fns else ""))
                for n_no, fn in enumerate(fns, start=1):
                    w(f"  - [ ] {num}.{n_no} `{fn}()` · `TODO-FN`")

                records.append({
                    "n": num, "path": rel, "lines": lines, "tag": tag,
                    "asm": bool(asm), "generators": gen,
                    "functions": fns,
                })
            w()

        area_rows.append((a_no, prefix, a_files, a_lines, a_funcs,
                          dict(sorted(a_tags.items()))))
        w()

    fh.close()

    # ---- summary, prepended ----------------------------------------------
    body = out_path.read_text(encoding="utf-8")
    head, _, rest = body.partition("**The function list is a floor")
    marker = "**The function list is a floor" + rest[:rest.index("\n\n") + 2]
    tail = rest[rest.index("\n\n") + 2:]

    summary = ["## Totals", "",
               "| # | area | files | lines | functions |",
               "|---:|---|---:|---:|---:|"]
    tf = tl = tn = 0
    for a_no, prefix, files, lines, funcs, _ in area_rows:
        summary.append(f"| {a_no} | `{prefix}` | {files:,} | {lines:,} | "
                       f"{funcs:,} |")
        tf += files
        tl += lines
        tn += funcs
    summary.append(f"| | **total** | **{tf:,}** | **{tl:,}** | **{tn:,}** |")
    summary += ["", "| tag | files |", "|---|---:|"]
    for tag, n in sorted(totals.items(), key=lambda kv: -kv[1]):
        summary.append(f"| `{tag}` | {n:,} |")
    summary += [""]

    # ---- indexes: the same ledger, sorted the ways it gets used ----------
    have_fns = [r for r in records if r.get("functions") is not None]

    def table(title, rows, note=""):
        summary.append(f"### {title}")
        summary.append("")
        if note:
            summary.append(note)
            summary.append("")
        summary.append("| item | path | lines | fn | tag |")
        summary.append("|---|---|---:|---:|---|")
        for r in rows:
            summary.append(
                f"| {r['n']} | `{r['path']}` | {r['lines']:,} | "
                f"{len(r.get('functions') or []):,} | `{r['tag']}` |")
        summary.append("")

    summary.append("## Indexes")
    summary.append("")
    summary.append("The same ledger, sorted the ways it actually gets used.")
    summary.append("")

    port = [r for r in have_fns if r["tag"] == "TODO-PORT"]
    table("The hundred largest files to port",
          sorted(port, key=lambda r: -r["lines"])[:100],
          "Where the work is, by line count. A 5,000-line C file is not "
          "five hundred\nten-line ones; these are the entries that need a "
          "plan of their own.")

    asm = [r for r in records if r["tag"] == "TODO-ASM"]
    table(f"All {len(asm):,} assembly files",
          sorted(asm, key=lambda r: -r["lines"]),
          "`docs/ASSEMBLY.md` divides these into what can become C++ and "
          "what cannot.\nNothing here is classified yet — that is the "
          "classification job, item by item.")

    dec = [r for r in records if r["tag"] == "TODO-DECIDE"]
    if dec:
        table(f"All {len(dec)} product decisions",
              sorted(dec, key=lambda r: r["path"]),
              "A decision about what PBSD *is* comes before a decision about "
              "how to port it.")

    gen = sorted((r for r in have_fns if r.get("generators", 0) >= 20),
                 key=lambda r: -r["generators"])[:60]
    if gen:
        summary.append("### Where the function list under-reports most")
        summary.append("")
        summary.append("Files with twenty or more macro invocations that "
                       "look like generators. The")
        summary.append("`fn` column below is a floor and in these files it "
                       "is a low one — arm64's")
        summary.append("`atomic.h` generates its entire interface and the "
                       "regex sees none of it.")
        summary.append("")
        summary.append("| item | path | lines | fn | generators |")
        summary.append("|---|---|---:|---:|---:|")
        for r in gen:
            summary.append(
                f"| {r['n']} | `{r['path']}` | {r['lines']:,} | "
                f"{len(r.get('functions') or []):,} | {r['generators']:,} |")
        summary.append("")

    big = sorted((r for r in have_fns if r.get("functions")),
                 key=lambda r: -len(r["functions"]))[:60]
    table("The sixty files with the most functions", big,
          "Each of these is a ledger of its own. `TODO-FN` lines below are "
          "one per\nfunction; a file with two hundred of them is a project.")

    summary += [""]

    out_path.write_text(head + marker + "\n".join(summary) + tail,
                        encoding="utf-8")

    json_path.write_text(
        json.dumps({"areas": [
            {"n": a, "area": p, "files": f, "lines": l, "functions": fn,
             "tags": t} for a, p, f, l, fn, t in area_rows],
            "totals": totals, "records": records}, indent=1) + "\n",
        encoding="utf-8")

    size = out_path.stat().st_size
    print(f"{out_path}: {tf:,} files, {tn:,} functions, "
          f"{size / 1e6:.1f} MB")
    print(f"{json_path}: machine-readable, "
          f"{json_path.stat().st_size / 1e6:.1f} MB")
    for tag, n in sorted(totals.items(), key=lambda kv: -kv[1]):
        print(f"  {tag:14s} {n:6,}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
