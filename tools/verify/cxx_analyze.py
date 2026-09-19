#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""clang's static analyser over the C++ tiers - KDE and hbsd/src C++.

The C analogue is tools/verify/analyze.py and this is deliberately its
twin: same CLI shape, same JSONL record schema, same ERROR accounting,
so tools/verify/confidence.py and tools/verify/report.py consume the
output of both without a line changed. Where a field name differs from
analyze.py's, this file is wrong and analyze.py is right.

WHAT IS DIFFERENT ABOUT C++, AND WHY THIS FILE EXISTS SEPARATELY
----------------------------------------------------------------
analyze.py derives its flags from the FreeBSD build - includes.py reads
sys/conf/files, the option index, the arch tables - because the C tree
has a build system that states them. The C++ tiers have two build
systems and this tool can only see one of them:

  KDE      cmake + extra-cmake-modules. It emits compile_commands.json
           when it configures. MEASURED 2026-09-17 in the analysis
           container: 0 of 12 KDE projects configure, all 12 die at
           find_package(ECM), and 0 compile_commands.json exist.
  hbsd C++ bmake, and mostly vendored LLVM whose real flags come from
           a cmake build that also runs TableGen. No .inc, no build.

So there are TWO flag sources and the record says which one it used:

    flagsrc = "compile_commands"   the project configured; these are
                                   the flags the build would use.
    flagsrc = "guess"              nobody configured anything; these
                                   are include paths inferred from the
                                   directory layout.

A finding from guessed flags is WEAKER EVIDENCE and the record carries
that, at record level (`flagsrc`) and on every finding (`guessed`),
because a finding gets copied out of its record and has to keep its
provenance when it does. Guessed flags mean the analyser read a
DIFFERENT PROGRAM from the one that ships: an unsupplied -D means a
#if is false, a synthesised config-kwin.h means every HAVE_* is
undefined, and the code behind those is not merely unchecked, it was
not there.

THE THREE STATES THIS FILE EXISTS TO KEEP APART
-----------------------------------------------
    not run                no record for the file at all. A scope with
                           no records is NOT a clean scope.
    ran, would not build   status ERROR, counted, inventoried by
                           --check-errors, and it drags VISIBILITY down
                           in confidence.py.
    ran, found nothing     status OK with findings == [].

A file that does not compile reports zero findings and is
indistinguishable from a clean one. libexec/rtld-elf/rtld.c did exactly
that in the C tier and the defect behind it cost thirty-seven boot runs.
On the C++ side the same failure is not one file, it is 1,386 of 1,392:
a run that printed "0 findings across the KDE tree" would be true and
would mean nothing. Hence:

EXIT CODES
    0   ran, at least one translation unit built, error gate passed
    1   --check-errors gate failed (an un-inventoried ERROR, or a
        stale exemption that compiles now)
    2   ran and built NOTHING, or a --scope matched no files at all.
        There is no finding count worth reading from such a run and
        the shell must be able to tell.

CHECKERS
--------
Chosen by what a failure MEANS, the same rule analyze.py uses. The C
set is inherited whole - a C++ translation unit still calls malloc,
still divides by zero, still reads uninitialised memory - and the
`cplusplus.*` package is added on top. Every name below was confirmed
present in `clang -cc1 -analyzer-checker-help` on clang 18.1.3 and all
but two were confirmed FIRING on a probe with the defect planted; see
CHECKER_EVIDENCE.

Excluded, and each exclusion is a POLICY CALL and not a coverage gap -
the difference being that a policy call is a decision about what counts
as a defect, while a coverage gap is code nobody looked at:

  deadcode.*            Dead stores are style. In a portability tree
                        half of them are a value assigned under one
                        #ifdef and read under another, so the checker
                        is measuring the preprocessor, not the code.
                        Inherited from analyze.py.
  security.insecureAPI.*  A 1990s codebase deliberately calls strcpy,
                        bcopy and mktemp. Flagging every one is a
                        rewrite proposal, not a finding. Inherited
                        from analyze.py.
  optin.*               Opt-in by upstream's own judgement, which is
                        upstream saying the false-positive rate is not
                        good enough to be on. optin.cplusplus.VirtualCall
                        fires on a pattern KDE's own code uses freely.
  alpha.*               Upstream marks several "Enable only for
                        development!" in the help text itself.
  nullability.*, osx.*, webkit.* Not this tree's languages or frameworks.

None of these is suppressed in a way that hides a file. They are
checkers not read; every translation unit they would have run on is
still OK or still ERROR in the record, so VISIBILITY is unaffected.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import plistlib
import re
import shlex
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

# ---------------------------------------------------------------- checkers

# Inherited from tools/verify/analyze.py. A C++ TU still does all of this.
C_CHECKERS = [
    "core.CallAndMessage", "core.DivideZero", "core.NonNullParamChecker",
    "core.NullDereference", "core.StackAddressEscape",
    "core.UndefinedBinaryOperatorResult", "core.VLASize",
    "core.uninitialized.ArraySubscript", "core.uninitialized.Assign",
    "core.uninitialized.Branch", "core.uninitialized.UndefReturn",
    "unix.Malloc", "unix.MallocSizeof", "unix.MismatchedDeallocator",
    "unix.cstring.BadSizeArg", "unix.cstring.NullArg",
]

# The C++ package. Present in clang 18.1.3's non-alpha list, which is
# the default-enabled set.
CXX_CHECKERS = [
    "cplusplus.InnerPointer",       # container inner pointer used after realloc
    "cplusplus.Move",               # use-after-move
    "cplusplus.NewDelete",          # double-free and use-after-free on new/delete
    "cplusplus.NewDeleteLeaks",     # leak of new'd memory
    "cplusplus.PlacementNew",       # placement new into too little storage
    "cplusplus.PureVirtualCall",    # pure virtual call during ctor/dtor
    "cplusplus.StringChecker",      # std::string misuse
    # core.BitwiseShift and core.uninitialized.NewArraySize exist in 18
    # and are not in analyze.py's C list because they did not exist when
    # it was written. Shifts by >= width and `new T[n]` with n undefined
    # are both defects, so they are in.
    "core.BitwiseShift",
    "core.uninitialized.NewArraySize",
]

CHECKERS = C_CHECKERS + CXX_CHECKERS

# What was actually demonstrated, rather than assumed. Written down
# because "the checker is in the list" and "the checker fires" are two
# different claims and this project has been burned by conflating them:
# analyze.py's docstring records a version that ran NOTHING and reported
# zero. tools/verify-equivalent probe is test_cxx_analyze.py.
CHECKER_EVIDENCE = {
    # confirmed firing on a planted defect, clang 18.1.3, 2026-09-17
    "fired_on_probe": [
        "cplusplus.NewDelete", "cplusplus.NewDeleteLeaks",
        "cplusplus.InnerPointer", "cplusplus.Move",
        "cplusplus.PlacementNew", "cplusplus.PureVirtualCall",
        "unix.MismatchedDeallocator", "core.NullDereference",
        "core.DivideZero", "core.UndefinedBinaryOperatorResult",
        "cplusplus.StringChecker",
    ],
    # present in the checker list, not exercised by the probe. NOT the
    # same as "works".
    "listed_not_probed": [
        "core.BitwiseShift",
        "core.uninitialized.NewArraySize", "core.CallAndMessage",
        "core.NonNullParamChecker", "core.StackAddressEscape",
        "core.VLASize", "core.uninitialized.ArraySubscript",
        "core.uninitialized.Assign", "core.uninitialized.Branch",
        "core.uninitialized.UndefReturn", "unix.Malloc",
        "unix.MallocSizeof", "unix.cstring.BadSizeArg",
        "unix.cstring.NullArg",
    ],
    # asked for and NOT AVAILABLE in clang 18. Recorded so that its
    # absence is a known hole rather than a silent one.
    # Asked for and NOT AVAILABLE as a reporting checker in clang 18.
    # Recorded so the absence is a KNOWN hole rather than a silent one:
    # a checker that is registered but says nothing is exactly the shape
    # of a coverage number that lies.
    "unavailable": {
        "cplusplus.SelfAssignment":
            "REGISTERED BUT SILENT in clang 18.1.3. clang-tidy lists "
            "clang-analyzer-cplusplus.SelfAssignment and clang accepts "
            "-analyzer-checker=cplusplus.SelfAssignment without "
            "complaint, but it is a MODELLING checker - it emits no "
            "diagnostic of its own and does not appear in "
            "-analyzer-checker-help. MEASURED: on a probe whose "
            "operator= deletes its buffer before copying, both the "
            "default set and the explicitly-enabled checker report "
            "NOTHING. cxx_tidy.py's bugprone-unhandled-self-assignment "
            "DOES report it, which is the whole argument for running "
            "the second instrument.",
    },
}

_TREE = Path(__file__).resolve().parents[2]
DEFAULT_ROOT = Path(os.environ.get("PBSD_ROOT", str(_TREE)))

# KDE projects: <root>/kde/frameworks/<name> or <root>/kde/<name>.
# hbsd C++: <root>/hbsd/src/...
SRC_REL = Path("hbsd") / "src"

# Temp directories this run made. Their NAMES are a run's own; only
# their contents were asked for. Normalised out of the flags digest so
# two sweeps of the same tree agree. Same reasoning as analyze.py's.
TMPDIR = re.compile(r"(/tmp/(?:pbsd|cxxshim)_[a-z]+_)[A-Za-z0-9_]+")

# Flags from a compile_commands.json entry that must NOT be replayed:
# they write files, or they name outputs that do not exist.
DROP_EXACT = {"-c", "-o", "-MD", "-MMD", "-MP", "-MT", "-MF", "-M", "-MM",
              "--", "-fdiagnostics-color=always"}
DROP_WITH_ARG = {"-o", "-MT", "-MF", "-MQ", "-Xclang"}


# -------------------------------------------------------------- path names

def record_path(abs_path: Path, root: Path) -> str:
    """The name a record calls a file, and the one convention here that
    is not analyze.py's.

    analyze.py names files relative to hbsd/src, because that is what
    docs/port_plan.json - the ledger confidence.py takes its DENOMINATOR
    from - names them. Keeping that is not a preference, it is the only
    way `confidence.py --scope lib/msun` finds these records.

    KDE is not under hbsd/src and has NO entry in port_plan.json at all
    (measured: 35,050 ledger records, 0 of them under kde/). So a KDE
    file is named relative to the repository root - "kde/kwin/src/x.cpp"
    - which is unambiguous, and confidence.py will score any kde/ scope
    as NO LEDGER ENTRIES until somebody adds them. That is the honest
    answer: the ledger does not know this tree exists, and a scope the
    ledger does not know cannot have a visibility fraction.
    """
    try:
        return abs_path.resolve().relative_to((root / SRC_REL).resolve()).as_posix()
    except ValueError:
        pass
    try:
        return abs_path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return str(abs_path)


def dedup_checker(name: str) -> str:
    """clang-tidy spells the analyser's checkers with a prefix.

    core.NullDereference and clang-analyzer-core.NullDereference are the
    same checker reporting the same defect; a merge pass that keyed on
    the raw name would count them twice and call that corroboration.
    """
    return name[len("clang-analyzer-"):] if name.startswith("clang-analyzer-") else name


def finding(path: str, line, col, checker: str, msg: str, *,
            guessed: bool, fn: str | None = None,
            fn_kind: str | None = None) -> dict:
    """One finding, in the shape report.py and confidence.py already read.

    `where` is EXACTLY "path:line" with no column. report.py's agree()
    matches it with re.fullmatch(r"([^\\s:]+):(\\d+)") and a column would
    make every C++ finding invisible to the cross-instrument agreement
    section - which is the section whose whole job is noticing when two
    instruments land on one line. The column lives in its own field.

    `key` is the stable identity asked of this tier: file, line, column
    and check-name. `dedup` is the same with the tool prefix stripped,
    so a clang-tidy record and a clang --analyze record of one root
    cause collapse to one entry in a later merge instead of voting twice.
    """
    rec = {
        "where": f"{path}:{line}",
        "checker": checker,
        "msg": msg,
        "col": col,
        "key": f"{path}:{line}:{col}:{checker}",
        "dedup": f"{path}:{line}:{col}:{dedup_checker(checker)}",
    }
    if guessed:
        # Travels with the finding, not only with the record: a finding
        # gets copied out of its record into a triage list and has to
        # keep knowing that the flags behind it were invented.
        rec["guessed"] = True
    if fn:
        rec["fn"] = fn
        if fn_kind and fn_kind != "function":
            rec["fn_kind"] = fn_kind
    return rec


# ------------------------------------------------------ flag source (a):
#                                                 compile_commands.json

def load_compile_commands(paths: list[str]) -> dict:
    """abs source path -> (argv, directory), from any number of databases.

    When cmake has configured, THIS is the answer and nothing here needs
    to guess. Measured in the analysis container: no database exists,
    because no project configures. On the user's machine with ECM and
    Qt6 installed, it will.
    """
    out: dict[str, tuple[list[str], str]] = {}
    for p in paths:
        fp = Path(p)
        if fp.is_dir():
            fp = fp / "compile_commands.json"
        if not fp.is_file():
            continue
        try:
            db = json.loads(fp.read_text(errors="replace"))
        except ValueError:
            continue
        for e in db:
            d = e.get("directory", "")
            f = e.get("file", "")
            src = str((Path(d) / f).resolve()) if not Path(f).is_absolute() \
                else str(Path(f).resolve())
            if "arguments" in e:
                argv = list(e["arguments"])
            else:
                argv = shlex.split(e.get("command", ""))
            out[src] = (argv, d)
    return out


def replay_flags(argv: list[str]) -> list[str]:
    """A compile command, minus the parts that build rather than parse."""
    out, skip = [], False
    for i, a in enumerate(argv):
        if skip:
            skip = False
            continue
        if i == 0:
            continue                    # the compiler itself
        if a in DROP_WITH_ARG:
            skip = True
            continue
        if a in DROP_EXACT or a.startswith("-o"):
            continue
        if a.endswith((".c", ".cc", ".cpp", ".cxx", ".C", ".m", ".mm")):
            continue                    # the source; we add it ourselves
        out.append(a)
    return out


# ------------------------------------------------------ flag source (b):
#                                                          the guess

_SHIM_ENV = "CXX_ANALYZE_SHIM"


def build_shim(root: Path) -> str:
    """The headers cmake WOULD have generated, synthesised.

    Two families, and the second is the one to be uneasy about.

    *_export.h is what GenerateExportHeader/ECMGenerateExportHeader
    writes: a handful of visibility and deprecation macros. Synthesising
    it is close to lossless - the macros expand to nothing in a build
    too, on the compiler this tool runs.

    config-<project>.h is what cmake writes after probing the system,
    and synthesising it EMPTY means every HAVE_* is undefined, so every
    `#if HAVE_FOO` block is not merely unanalysed, it is not compiled.
    That is a different program. It is done anyway, because the
    alternative is that the file is ERROR and 100% unseen rather than
    partly seen, but every TU that touches the shim is flagsrc="guess"
    and every finding out of it carries guessed=True.

    MEASURED, 2026-09-17, on the 209 KDE .cpp that do not name a Qt
    header directly or through their own .h: without the shim 3 of 209
    parse, with it 6 of 209. The shim moved 136 "missing generated
    header" failures to "missing QObject". It does not open the door,
    it shows you the next one.
    """
    d = Path(tempfile.mkdtemp(prefix="cxxshim_gen_"))
    exp = re.compile(r'#\s*include\s*[<"]([A-Za-z0-9_./-]+_export\.h)[>"]')
    cfg = re.compile(r'#\s*include\s*[<"](config-[A-Za-z0-9_.-]+\.h)[>"]')
    names: set[str] = set()
    kde = root / "kde"
    if kde.is_dir():
        for f in list(kde.rglob("*.cpp")) + list(kde.rglob("*.h")):
            try:
                t = f.read_text(errors="replace")
            except OSError:
                continue
            names.update(exp.findall(t))
            names.update(cfg.findall(t))
    for n in sorted(names):
        p = d / n
        p.parent.mkdir(parents=True, exist_ok=True)
        base = Path(n).name
        if base.startswith("config-"):
            p.write_text("#pragma once\n/* SHIM: cmake generates this. "
                         "Every HAVE_* is UNDEFINED here, so every "
                         "#if HAVE_* block is absent from what was "
                         "analysed. */\n")
            continue
        P = base[:-len("_export.h")].upper().replace("-", "_")
        body = ["#pragma once", f"/* SHIM for {n} */"]
        for s in ("EXPORT", "NO_EXPORT", "DEPRECATED", "DEPRECATED_EXPORT",
                  "DEPRECATED_NO_EXPORT", "NO_DEPRECATED"):
            body.append(f"#define {P}_{s}")
        for s in ("DEPRECATED_VERSION", "ENUMERATOR_DEPRECATED_VERSION"):
            body.append(f"#define {P}_{s}(x, y, t)")
        for s in ("DEPRECATED_VERSION_BELATED",
                  "ENUMERATOR_DEPRECATED_VERSION_BELATED"):
            body.append(f"#define {P}_{s}(x, y, a, b, t)")
        body += [f"#define {P}_BUILD_DEPRECATED_SINCE(x, y) 1",
                 f"#define {P}_EXCLUDE_DEPRECATED_BEFORE_AND_AT 0",
                 f"#define {P}_DEPRECATED_WARNINGS_SINCE 0"]
        p.write_text("\n".join(body) + "\n")
    return str(d)


_INC_CACHE: dict = {}


def _header_dirs(rootdir: Path) -> list[str]:
    """Every directory under a project that holds a header.

    Blunt, and blunt is the point: with no build system to ask, the only
    fact available is where the files are. Cached per project because
    the walk is the expensive part.
    """
    key = str(rootdir)
    if key not in _INC_CACHE:
        dirs = set()
        if rootdir.is_dir():
            for pat in ("*.h", "*.hpp", "*.hxx", "*.inc"):
                for h in rootdir.rglob(pat):
                    dirs.add(str(h.parent))
        _INC_CACHE[key] = ["-I" + d for d in sorted(dirs)]
    return _INC_CACHE[key]


def kde_project_of(abs_path: Path, root: Path) -> Path | None:
    try:
        rel = abs_path.resolve().relative_to((root / "kde").resolve()).parts
    except ValueError:
        return None
    if not rel:
        return None
    if rel[0] == "frameworks" and len(rel) > 1:
        return root / "kde" / "frameworks" / rel[1]
    return root / "kde" / rel[0]


def guess_flags(abs_path: Path, root: Path) -> list[str]:
    """Best-effort include paths, inferred from the directory layout.

    This is flag source (b) and everything it produces is marked. It
    exists because the alternative to a guess is not a better answer, it
    is no answer - and no answer reports zero findings and looks clean.

    MEASURED with this exact rule, 2026-09-17, clang 18.1.3:
      KDE     6 of 1,392 .cpp parse (census, not a sample). The other
              1,386 need Qt6 headers that are not installed.
      hbsd    76 of 200 sampled C++ files parse (38%). The wall behind
              that is TableGen .inc output - llvm/IR/Attributes.inc and
              friends - which no include path can find because the
              build generates it.
    """
    src = root / SRC_REL
    f: list[str] = [f"-I{abs_path.parent}"]
    shim = os.environ.get(_SHIM_ENV)
    if shim:
        f.append(f"-I{shim}")

    proj = kde_project_of(abs_path, root)
    if proj is not None:
        # Every header directory in the owning framework, then every
        # header directory in the sibling frameworks: KDE code includes
        # <KConfigGroup> from kio, and in a real build that comes from
        # an INSTALLED framework, which this tree does not have.
        f += _header_dirs(proj)
        fw = root / "kde" / "frameworks"
        if fw.is_dir():
            for sib in sorted(fw.iterdir()):
                if sib.is_dir() and sib != proj:
                    f += _header_dirs(sib)
        return f

    try:
        rel = abs_path.resolve().relative_to(src.resolve()).as_posix()
    except ValueError:
        return f
    B = src / "contrib" / "llvm-project"
    parts = rel.split("/")
    if rel.startswith("contrib/llvm-project/libcxx/"):
        # libc++ compiles against ITS OWN headers, and needs the two
        # files cmake generates that lib/libc++ keeps checked in.
        f += [f"-I{src / 'lib' / 'libc++'}",
              f"-I{B / 'libcxx' / 'include'}", f"-I{B / 'libcxx' / 'src'}"]
    elif (rel.startswith("contrib/llvm-project/")
          or rel.startswith("lib/clang/") or rel.startswith("usr.bin/clang/")):
        # The -I that lib/clang/*.mk states outright; see
        # includes.llvm_shim(), which says the same thing for the
        # fourteen files FreeBSD actually builds. The vendored sources
        # themselves are not built by FreeBSD at all, so there is no
        # .mk to read for them and this is the layout instead.
        f.append(f"-I{src / 'lib' / 'clang' / 'include'}")
        for s in ("llvm", "clang", "lld", "lldb", "compiler-rt", "openmp",
                  "libunwind", "polly", "mlir"):
            for sub in ("include", "source", "src"):
                d = B / s / sub
                if d.is_dir():
                    f.append(f"-I{d}")
        if len(parts) > 2:
            f.append(f"-I{B / parts[2]}")
            if (B / parts[2] / "lib").is_dir():
                f.append(f"-I{B / parts[2] / 'lib'}")
            # ...and the file's own directory chain up to the subproject
            # root: LLVM backends include "MCTargetDesc/X86BaseInfo.h"
            # relative to lib/Target/X86.
            stop, p = B / parts[2], abs_path.parent
            while p != stop and stop in p.parents:
                f.append(f"-I{p}")
                p = p.parent
    elif rel.startswith("contrib/googletest"):
        for s in ("googletest", "googlemock"):
            for sub in ("include", ""):
                d = src / "contrib" / "googletest" / s / sub
                if d.is_dir():
                    f.append(f"-I{d}")
    else:
        for top in ("contrib/kyua", "contrib/atf", "contrib/lutok",
                    "contrib/opencsd", "lib/libdevdctl", "lib/libutil++"):
            if rel.startswith(top):
                f.append(f"-I{src / top}")
                f += _header_dirs(src / top)
    return f


# --------------------------------------------------------------- the run

DIALECT = ["-std=c++20"]


def analyze(job: dict) -> dict:
    """One translation unit. Never returns "no findings" for a TU it
    could not build - that case is status ERROR and it is counted.

    The analyser is run with its DEFAULT checkers and the OUTPUT is
    filtered, exactly as analyze.py does and for the reason its
    docstring gives: -analyzer-disable-all-checks plus an explicit
    -analyzer-checker list runs NOTHING and reports zero, which is the
    failure this whole tier is built to make impossible. Selecting from
    output cannot be silently empty, and test_cxx_analyze.py fails if
    the probe with nine planted defects comes back quiet.
    """
    src = Path(job["src"])
    root = Path(job["root"])
    rel = job["rel"]
    guessed = job["flagsrc"] == "guess"
    flags = list(job["flags"])
    digest = hashlib.sha256(
        "\n".join(TMPDIR.sub(r"\1<tmp>", x) for x in flags).encode()
    ).hexdigest()[:12]
    base = {"file": rel, "flags": digest, "flagsrc": job["flagsrc"]}

    plist = tempfile.NamedTemporaryFile(prefix="cxx_diag_", suffix=".plist",
                                        delete=False)
    plist.close()
    # plist-MULTI-FILE, not plist: plain plist DROPS every report whose
    # path leaves the main source and says nothing about it. In C++ that
    # is most of them - the leak is reported in the header that defines
    # the container. analyze.py lost 13 of 255 findings that way in C.
    cmd = ["clang++", "--analyze", "-Xclang",
           "-analyzer-output=plist-multi-file",
           *flags, str(src), "-o", plist.name]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=job["timeout"], cwd=job.get("cwd", "/tmp"))
    except subprocess.TimeoutExpired:
        Path(plist.name).unlink(missing_ok=True)
        # TIMEOUT is not OK and it is not ERROR. It is "the instrument
        # was still thinking", which is a third thing and stays a third
        # thing.
        return {**base, "status": "TIMEOUT", "findings": []}
    except OSError as e:
        Path(plist.name).unlink(missing_ok=True)
        return {**base, "status": "ERROR", "detail": str(e), "findings": []}

    if p.returncode != 0 and "error:" in p.stderr:
        Path(plist.name).unlink(missing_ok=True)
        if not job.get("retried_dialect"):
            # The one retry worth having. KDE declares C++20; vendored
            # LLVM in this tree is C++17 and some of it does not parse
            # as C++20. Trying the other dialect is cheap and a file
            # that already failed has nothing to lose - same rule as
            # analyze.py's architecture retry.
            alt = "-std=c++17" if "-std=c++20" in flags else "-std=c++20"
            nf = [x for x in flags if not x.startswith("-std=")] + [alt]
            r = analyze(dict(job, flags=nf, retried_dialect=True))
            if r["status"] != "ERROR":
                r["std"] = alt
                return r
        return {**base, "status": "ERROR",
                "detail": p.stderr.strip()[-300:], "findings": []}

    wanted = set(CHECKERS)
    pl = Path(plist.name)
    raw = pl.read_bytes() if pl.is_file() else b""
    pl.unlink(missing_ok=True)
    # clang writes the file only when it has something to say, and a
    # unit it gave up on leaves it empty rather than reporting that.
    doc = plistlib.loads(raw) if raw.lstrip().startswith(b"<?xml") else {}
    files = doc.get("files", [])
    out = []
    for d in doc.get("diagnostics", []):
        if d.get("check_name") not in wanted:
            continue
        loc = d.get("location", {})
        name = files[loc["file"]] if loc.get("file") is not None else ""
        where = record_path(Path(name), root) if name else rel
        out.append(finding(where, loc.get("line"), loc.get("col"),
                           d["check_name"], d.get("description", ""),
                           guessed=guessed, fn=d.get("issue_context"),
                           fn_kind=d.get("issue_context_kind")))
    return {**base, "status": "OK", "findings": out}


# --------------------------------------------------------------- inventory

def load_expected(path: str) -> tuple[dict, dict]:
    """The C++ analogue of tools/verify/expected_errors.py.

    Same contract: an ERROR that is on the record is a decision, an
    ERROR that is not is a new hole, and an entry that compiles now is a
    stale exemption. Kept as JSON rather than Python because this tier's
    inventory is currently 1,386 entries produced by a measurement, not
    a dozen produced by reading Makefiles, and a machine writes it.

    {"files": {"<rel>": "<why>"}, "prefixes": {"<rel>/": "<why>"}}
    """
    fp = Path(path)
    if not fp.is_file():
        return ({}, {})
    try:
        d = json.loads(fp.read_text(errors="replace"))
    except ValueError:
        return ({}, {})
    return (d.get("files") or {}, d.get("prefixes") or {})


def prefix_of(rel: str, prefixes: dict) -> str | None:
    for p in sorted(prefixes, key=len, reverse=True):
        if rel.startswith(p):
            return p
    return None


def summarise_errors(path: str, top: int = 12) -> None:
    """Why the translation units that did not compile did not compile.

    Verbatim in intent from analyze.py: an ERROR contributes nothing and
    says so nowhere, and five thousand individual messages nobody
    bucketed is how a corpus sits at a fifth of its size unnoticed. On
    this tier the buckets are the deliverable - "missing header: QObject"
    at the top of the list IS the survey result.
    """
    import collections
    buckets: collections.Counter = collections.Counter()
    example: dict[str, str] = {}
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                if not line.strip():
                    continue
                try:
                    r = json.loads(line)
                except ValueError:
                    continue
                if r.get("status") != "ERROR":
                    continue
                d = r.get("detail") or ""
                m = re.search(r"fatal error: '([^']+)' file not found", d)
                if m:
                    key = f"missing header: {m.group(1)}"
                else:
                    m2 = re.search(r"error: (.{0,72})", d)
                    key = (m2.group(1) if m2 else d[-72:]).strip()
                    key = re.sub(r"\s+", " ", key)
                buckets[key] += 1
                example.setdefault(key, r["file"])
    except OSError:
        return
    if not buckets:
        return
    print(f"\n== why {sum(buckets.values())} translation unit(s) did not "
          "compile")
    print("   A file that does not compile contributes no findings and "
          "says so nowhere.")
    for key, n in buckets.most_common(top):
        print(f"  {n:5d}  {key}")
        print(f"         e.g. {example[key]}")
    rest = len(buckets) - min(top, len(buckets))
    if rest > 0:
        print(f"  ...and {rest} more distinct reason(s)")


def analyzer_version() -> str:
    try:
        out = subprocess.run(["clang++", "--version"], capture_output=True,
                             text=True, timeout=20)
        return (out.stdout or out.stderr).splitlines()[0].strip()
    except (OSError, IndexError, subprocess.SubprocessError) as e:
        return f"unknown ({e})"


CXX_SUFFIXES = (".cpp", ".cc", ".cxx", ".C")
DEFAULT_SCOPES = ["kde/frameworks", "kde/kwin", "kde/plasma-desktop"]


def collect(root: Path, scopes: list[str], db: dict,
            limit: int | None, timeout: int = 300) -> tuple[list[dict], dict]:
    """Translation units, and how many each scope contributed.

    The per-scope count is not decoration. A scope that matched NO FILES
    produces no records, and no records is what "never run" looks like -
    so the two have to be told apart here, where the difference is still
    knowable, rather than downstream where it is not.
    """
    jobs: list[dict] = []
    per_scope: dict[str, int] = {}
    for s in scopes:
        d = (root / s)
        n = 0
        files: list[Path] = []
        if d.is_file():
            files = [d]
        else:
            for suf in CXX_SUFFIXES:
                files += list(d.rglob("*" + suf))
        for f in sorted(files):
            key = str(f.resolve())
            if key in db:
                argv, cwd = db[key]
                flags, srcname, cwd_ = replay_flags(argv), "compile_commands", cwd
            else:
                flags, srcname, cwd_ = (guess_flags(f, root) + DIALECT,
                                        "guess", "/tmp")
            jobs.append({"src": str(f), "rel": record_path(f, root),
                         "root": str(root), "flags": flags,
                         "flagsrc": srcname, "cwd": cwd_,
                         "timeout": timeout})
            n += 1
        per_scope[s] = n
    if limit:
        jobs = jobs[:limit]
    return jobs, per_scope


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append",
                    help="path prefix under --root; repeatable. "
                         "default=None deliberately: argparse's append "
                         "ADDS to a non-empty default, which is how a "
                         "narrowed run silently did the whole tree.")
    ap.add_argument("--root", default=str(DEFAULT_ROOT))
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--timeout", type=int, default=300)
    ap.add_argument("--out", default="cxx_analyze_results.jsonl")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--compile-commands", action="append", default=[],
                    help="a compile_commands.json, or a build directory "
                         "holding one; repeatable. A TU found here is "
                         "flagsrc=compile_commands; every other TU is "
                         "flagsrc=guess and says so in its record.")
    ap.add_argument("--no-shim", action="store_true",
                    help="do not synthesise the cmake-generated headers. "
                         "Raises the ERROR count and lowers VISIBILITY, "
                         "which is the honest reading when you do not "
                         "want findings from a program cmake never made.")
    ap.add_argument("--expected", default=str(
        Path(__file__).resolve().parent / "cxx_expected_errors.json"))
    ap.add_argument("--check-errors", action="store_true",
                    help="fail if a translation unit does not compile and "
                         "is not in the expected-errors inventory, or if "
                         "one listed there compiles now. An ERROR "
                         "contributes no findings and reads as clean in "
                         "every total.")
    args = ap.parse_args(argv)

    root = Path(args.root).resolve()
    if not root.is_dir():
        print(f"FAIL  --root {root} is not a directory. Nothing was run, "
              f"and nothing being run is not the same as nothing being "
              f"wrong.")
        return 2

    scopes = args.scope or DEFAULT_SCOPES
    db = load_compile_commands(args.compile_commands)
    if not args.no_shim:
        os.environ[_SHIM_ENV] = build_shim(root)

    jobs, per_scope = collect(root, scopes, db, args.limit, args.timeout)
    empty = [s for s, n in per_scope.items() if n == 0]
    nguess = sum(1 for j in jobs if j["flagsrc"] == "guess")

    print(f"{len(jobs)} translation unit(s) to analyse across "
          f"{len(scopes)} scope(s)")
    for s in scopes:
        print(f"    {per_scope.get(s, 0):6d}  {s}"
              + ("   <-- MATCHED NO FILES" if per_scope.get(s, 0) == 0 else ""))
    print(f"  flag source: {len(jobs) - nguess} from compile_commands.json, "
          f"{nguess} GUESSED")
    if nguess and not db:
        print("  no compile_commands.json was supplied or found. Every")
        print("  finding below is from invented flags and is weaker")
        print("  evidence; every record says flagsrc=guess.")

    counts, nfind, t0 = {}, 0, time.time()
    with open(args.out, "w") as fh:
        # First record: what produced these numbers. Marked "_meta" so
        # every reader that iterates findings skips it - it has no
        # "findings" key. confidence.py and report.py both already do.
        fh.write(json.dumps({
            "_meta": True,
            "tool": "cxx_analyze.py",
            "analyzer": analyzer_version(),
            "scopes": scopes,
            "scope_units": per_scope,
            "scopes_matching_nothing": empty,
            "units": len(jobs),
            "guessed_units": nguess,
            "checkers": CHECKERS,
            "checkers_unavailable": CHECKER_EVIDENCE["unavailable"],
            "path_base": "hbsd/src for files under it, repository root "
                         "otherwise (kde/ is not in docs/port_plan.json)",
        }) + "\n")
        if jobs:
            with ProcessPoolExecutor(args.jobs) as ex:
                futs = [ex.submit(analyze, j) for j in jobs]
                for i, fut in enumerate(as_completed(futs), 1):
                    r = fut.result()
                    counts[r["status"]] = counts.get(r["status"], 0) + 1
                    nfind += len(r["findings"])
                    fh.write(json.dumps(r) + "\n")
                    for f in r["findings"]:
                        mark = " (GUESSED FLAGS)" if f.get("guessed") else ""
                        print(f"  {f['where']}  [{f['checker']}]{mark}\n"
                              f"      {f['msg'][:110]}", flush=True)
                    if i % 200 == 0 or i == len(jobs):
                        print(f"  [{i}/{len(jobs)}] "
                              f"{i / max(1e-9, time.time() - t0):.1f}/s  "
                              f"findings={nfind}  "
                              + "  ".join(f"{k}={v}"
                                          for k, v in sorted(counts.items())),
                              flush=True)

    ok = counts.get("OK", 0)
    other = ", ".join(f"{k}={v}" for k, v in sorted(counts.items())
                      if k != "OK") or "none"
    print(f"\n{ok} translation unit(s) BUILT, {len(jobs) - ok} did not "
          f"({other})")
    print(f"{nfind} finding(s) across the {ok} that built")
    for k, v in sorted(counts.items()):
        print(f"  {k:8s} {v}")
    if counts.get("ERROR"):
        summarise_errors(args.out)

    rc = 0

    # An ERROR is invisible in every total printed above: no findings,
    # and a file that reports none looks exactly like one that is clean.
    if args.check_errors:
        files_x, prefixes_x = load_expected(args.expected)
        seen, erred = set(), set()
        with open(args.out, encoding="utf-8") as fh:
            for line in fh:
                r = json.loads(line)
                if r.get("_meta"):
                    continue
                seen.add(r["file"])
                if r.get("status") == "ERROR":
                    erred.add(r["file"])
        unexpected = sorted(e for e in erred
                            if e not in files_x
                            and prefix_of(e, prefixes_x) is None)
        stale = sorted((set(files_x) & seen) - erred)
        absorbed: dict[str, int] = {}
        looked: dict[str, int] = {}
        for f in seen:
            pre = prefix_of(f, prefixes_x)
            if pre:
                looked[pre] = looked.get(pre, 0) + 1
                if f in erred:
                    absorbed[pre] = absorbed.get(pre, 0) + 1
        dead = sorted(p for p in looked if not absorbed.get(p))
        for f in unexpected[:40]:
            print(f"FAIL  {f} does not compile and is not in the inventory")
        if len(unexpected) > 40:
            print(f"FAIL  ...and {len(unexpected) - 40} more")
        for f in stale[:40]:
            print(f"FAIL  {f} compiles now; its entry is stale "
                  f"({files_x[f]})")
        for p_ in dead:
            print(f"FAIL  every file under {p_} compiles now; its prefix "
                  f"entry is stale ({prefixes_x[p_]})")
        for p_ in sorted(absorbed):
            print(f"      {absorbed[p_]:4d} of {looked[p_]} under {p_} "
                  f"are on the record as not-built")
        if unexpected or stale or dead:
            print(f"\n      {args.expected} is the inventory.")
            print("      A file that will not compile reports zero findings")
            print("      and is indistinguishable from a clean one.")
            rc = 1
        elif erred:
            print(f"\nok    all {len(erred)} ERROR translation unit(s) are "
                  f"on the record")

    # The loudest case, and the one this file exists for.
    if empty:
        print(f"\nFAIL  {len(empty)} scope(s) matched NO FILES: "
              f"{', '.join(empty)}")
        print("      That is NOT RUN. It is not a clean scope and it must")
        print("      not be reported as one.")
        return 2
    if jobs and ok == 0:
        print(f"\nFAIL  0 translation units BUILT, {len(jobs)} ERROR.")
        print("      There is no finding count to read from this run.")
        print("      VISIBILITY is zero. 'No findings' here means 'no")
        print("      instrument', and the two must never be confused.")
        return 2
    return rc


if __name__ == "__main__":
    sys.exit(main())
