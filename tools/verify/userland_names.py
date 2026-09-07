#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which userland sources the build names, asked of the build itself.

sweep_report's `--unlisted' grouping had one authority for the question
"does anything build this file": `sys/conf/files*' plus the module
Makefiles.  That authority is the *kernel's*.  Asked about
lib/libc/gen/getcwd.c it answers "nothing in the build names it", which
is not a finding about lib/libc - it is the kernel build being asked
about a file it has no opinion on.  Every one of the libs shard's 57
unlisted ERRORs landed in that bucket, which made the bucket say
nothing at all.

Userland has no `files' list; the answer lives in each directory's
Makefile, in SRCS and .PATH, after bsd.lib.mk and bsd.prog.mk and the
component's Makefile.inc chain have had their say.  Re-implementing that
resolution is how a second place for the answer to be wrong gets built,
so this does not re-implement it: it runs bmake over the tree's own
share/mk and reads SRCS and .PATH back out.

    bmake -m <src>/share/mk -V SRCS -V .PATH MACHINE=... MACHINE_ARCH=...

Each SRCS entry is then resolved against .PATH in order, the way bmake
resolves it, and the ones that land on a file in the tree are the
answer.  Names that resolve to nothing are generated sources (yacc's
.c, rpcgen's .h, a Makefile rule's output); they are not on disk, so
they cannot be a sweep record's path, and dropping them costs nothing.

The index is per architecture for the same reason the kernel's is: a
Makefile.inc under lib/libc/amd64 is read only when MACHINE_ARCH says
amd64, and lib/libc/aarch64/gen/getcontextx.c is named by no
architecture but its own.  `names()' unions the six.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import functools
import json
import os
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE.parent.parent / "hbsd" / "src"
SHARE_MK = SRC / "share" / "mk"
OBJDIR = os.environ.get("PBSD_BMAKE_OBJDIR", "/tmp/pbsd_bmake_obj")

# The same six the sweep analyses, spelled the way includes.py spells
# them, so a caller can hand either table's key to either.
MACHINE_OF = {
    "amd64":     ("amd64", "amd64", "amd64"),
    "i386":      ("i386", "i386", "i386"),
    "aarch64":   ("aarch64", "aarch64", "arm64"),
    "armv7":     ("arm", "armv7", "arm"),
    "powerpc64": ("powerpc", "powerpc64", "powerpc"),
    "riscv64":   ("riscv", "riscv64", "riscv"),
}

# The directories that hold programs and libraries.  `share' and
# `etc' hold neither; `sys' has its own authority and must not be
# answered for here, or a kernel source with no Makefile would come
# back "nothing names it" from a tool that never looked at files*.
# `contrib' is not a scope of its own: its sources are named by the
# lib and usr.bin Makefiles that .PATH into them, and its own Makefiles
# are upstream's, which bmake reads as something other than what they
# are.  `sys' is not here either - the kernel's authority answers for it.
SCOPES = ("lib", "libexec", "bin", "sbin", "usr.bin", "usr.sbin",
          "kerberos5", "cddl", "rescue", "stand", "tests",
          "secure", "games", "include")

SUFFIXES = (".c", ".cc", ".cpp", ".cxx", ".S", ".s", ".m", ".y", ".l")


def makefile_dirs(src: Path = SRC) -> list[Path]:
    """Every directory under the userland scopes with a Makefile."""
    out = []
    for scope in SCOPES:
        top = src / scope
        if not top.is_dir():
            continue
        for dirpath, dirnames, filenames in os.walk(top):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            if "Makefile" in filenames:
                out.append(Path(dirpath))
    return sorted(out)


def _bmake(d: Path, arch: str, want: list[str], src: Path,
           timeout: int) -> list[str] | None:
    cpuarch, marcH, mach = MACHINE_OF.get(arch, MACHINE_OF["amd64"])
    cmd = ["bmake", "-m", str(SHARE_MK)]
    for v in want:
        cmd += ["-V", v]
    cmd += [f"MACHINE={mach}", f"MACHINE_ARCH={marcH}",
            f"MACHINE_CPUARCH={cpuarch}", f"SRCTOP={src}"]
    # bmake creates an object directory as a side effect of reading a
    # Makefile that has one - 15,313 of them under /usr/obj the first
    # time this ran over the tree. MAKEOBJDIRPREFIX puts them somewhere
    # disposable instead; MK_AUTO_OBJ=no does not stop it.
    env = dict(os.environ, MAKEOBJDIRPREFIX=OBJDIR)
    try:
        p = subprocess.run(cmd, cwd=d, capture_output=True, text=True,
                           timeout=timeout, env=env)
    except (subprocess.TimeoutExpired, OSError):
        return None
    lines = p.stdout.split("\n")
    if len(lines) < len(want):
        return None
    return lines[:len(want)]


def ask(d: Path, arch: str, src: Path = SRC, timeout: int = 40
        ) -> tuple[list[str], list[str]]:
    """(the names this directory builds, .PATH).

    A directory with one program says so in SRCS, and one -V reads it.
    A directory with several says so in PROGS, and its sources are in
    SRCS.<prog> - or, when that is unset, bsd.prog.mk's default of
    <prog>.c, or <prog>.cc for a C++ one.  Asking only SRCS is why the
    first reading of lib/libc/tests/gen came back naming nothing at
    all while sitting on fifty test programs.

    A Makefile that will not parse - one that .includes a file the
    build generates, say - returns nothing rather than raising: the
    question is what the build names, and a Makefile bmake cannot read
    names nothing that can be checked.  `build()' counts those, so a
    large number is visible rather than silently shrinking the answer.
    """
    got = _bmake(d, arch, ["SRCS", ".PATH", "PROGS", "PROGS_CXX"],
                 src, timeout)
    if got is None:
        return [], []
    srcs, path, progs, progs_cxx = (g.split() for g in got)
    if progs:
        cxx = set(progs_cxx)
        per = _bmake(d, arch, [f"SRCS.{p}" for p in progs], src, timeout)
        for i, prog in enumerate(progs):
            own = per[i].split() if per and i < len(per) else []
            srcs += own or [prog + (".cc" if prog in cxx else ".c")]
    return srcs, path


def resolve(srcs: list[str], path: list[str], d: Path,
            src: Path = SRC) -> set[str]:
    """The tree-relative sources those names resolve to.

    bmake looks a name up in .PATH in order and takes the first hit;
    the first entry is `.', the directory itself.  A name that hits
    nothing is generated and is dropped.  A name still carrying a `$'
    is one bmake could not expand, which is not a file either.
    """
    dirs = [str(d)] + [p for p in path if p != "."]
    out = set()
    for name in srcs:
        if "$" in name or not name.endswith(SUFFIXES):
            continue
        for pdir in dirs:
            cand = os.path.join(pdir, name)
            if os.path.isfile(cand):
                try:
                    out.add(str(Path(cand).resolve().relative_to(src)))
                except ValueError:
                    pass          # outside the tree; not ours to name
                break
    return out


def build(arch: str, src: Path = SRC, jobs: int = 8
          ) -> tuple[set[str], list[str]]:
    """(sources this architecture's build names, Makefiles bmake refused)

    The refusals are returned rather than counted so that a caller can
    see whether they matter: 94 of them sound alarming until they turn
    out to be libclang_rt, ofed and kerberos5, none of which any sweep
    scope has yet reached.
    """
    dirs = makefile_dirs(src)
    named: set[str] = set()
    failed: list[str] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        futs = {ex.submit(ask, d, arch, src): d for d in dirs}
        for f in concurrent.futures.as_completed(futs):
            d = futs[f]
            srcs, path = f.result()
            if not srcs and not path:
                failed.append(str(d.relative_to(src)))
                continue
            named |= resolve(srcs, path, d, src)
    return named, sorted(failed)


def ask_incs(d: Path, arch: str, src: Path = SRC, timeout: int = 40
             ) -> dict[str, str]:
    """{installed path: source file} for the headers this directory installs.

    A program compiles against <devstat.h>, <jail.h>, <netgraph.h>,
    <security/pam_appl.h> - headers that are in the tree but not beside
    the program, and that the real build reaches through /usr/include
    because someone installed them there first. The analyser has no
    installed tree, so it has to be told, and the Makefile that installs
    each one says exactly where it goes:

        INCS=       devstat.h
        INCSDIR=    ${INCLUDEDIR}

    A directory can install more than one group - include/rpcsvc has
    INCSGROUPS= INCS RPCHDRS, and RPCHDRS goes to ${INCLUDEDIR}/rpc, not
    ${INCLUDEDIR}/rpcsvc - so the groups are read out of INCSGROUPS
    rather than INCS being assumed to be all of them.

    The name is resolved against .PATH like any other, because a header
    listed here need not live in the directory that installs it. What
    this does NOT handle is INCSNAME, which installs a file under a
    different name than it has on disk; there are few of those and they
    would come back as a header still not found rather than as a wrong
    one.
    """
    got = _bmake(d, arch, ["INCSGROUPS", ".PATH", "INCLUDEDIR"], src, timeout)
    if got is None:
        return {}
    groups = got[0].split() or ["INCS"]
    path, incdir = got[1].split(), got[2].strip() or "/usr/include"
    want: list[str] = []
    for g in groups:
        want += [g, g + "DIR"]
    vals = _bmake(d, arch, want, src, timeout)
    if vals is None:
        return {}
    dirs = [str(d)] + [x for x in path if x != "."]
    out: dict[str, str] = {}
    for i in range(0, len(want), 2):
        names = vals[i].split()
        where = vals[i + 1].strip() or incdir
        if not where.startswith(incdir):
            continue          # installed outside /usr/include; not a
        sub = where[len(incdir):].strip("/")   # header a program includes
        for n in names:
            if "$" in n or not n.endswith((".h", ".hh", ".hpp")):
                continue
            # The name can carry a directory of its own -
            # lib/libpam/libpam lists `security/pam_appl.h' and installs
            # it into ${INCLUDEDIR}/security, so the installed path is
            # security/pam_appl.h and not security/security/pam_appl.h.
            base = os.path.basename(n)
            for pdir in dirs:
                cand = os.path.join(pdir, n)
                if os.path.isfile(cand):
                    out[f"{sub}/{base}" if sub else base] = cand
                    break
    return out


def build_incs(arch: str, src: Path = SRC, jobs: int = 8) -> dict[str, str]:
    """{installed path: source file} for the whole tree, this architecture."""
    out: dict[str, str] = {}
    dirs = makefile_dirs(src)
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        for got in ex.map(lambda x: ask_incs(x, arch, src), dirs):
            for k, v in got.items():
                out.setdefault(k, v)
    return out


def incs_cache_path(arch: str) -> Path:
    return Path(os.environ.get("PBSD_CACHE", "/tmp")) / \
        f"pbsd_userland_incs_{arch}.json"


@functools.lru_cache(maxsize=None)
def installed_headers(arch: str = "amd64",
                      refresh: bool = False) -> dict[str, str]:
    p = incs_cache_path(arch)
    if not refresh and p.is_file():
        try:
            return json.loads(p.read_text())
        except (ValueError, OSError):
            pass
    out = build_incs(arch)
    try:
        p.write_text(json.dumps(out, indent=0, sort_keys=True))
    except OSError:
        pass
    return out


def cache_path(arch: str) -> Path:
    return Path(os.environ.get("PBSD_CACHE", "/tmp")) / \
        f"pbsd_userland_names_{arch}.json"


@functools.lru_cache(maxsize=None)
def for_arch(arch: str, refresh: bool = False) -> frozenset[str]:
    p = cache_path(arch)
    if not refresh and p.is_file():
        try:
            return frozenset(json.loads(p.read_text()))
        except (ValueError, OSError):
            pass
    named, _ = build(arch)
    try:
        p.write_text(json.dumps(sorted(named)))
    except OSError:
        pass
    return frozenset(named)


@functools.lru_cache(maxsize=None)
def names(arches: tuple[str, ...] = tuple(MACHINE_OF)) -> frozenset[str]:
    """Every source any of these architectures' builds names."""
    out: set[str] = set()
    for a in arches:
        out |= for_arch(a)
    return frozenset(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--arch", action="append", default=[],
                    help="architecture (repeatable; default all six)")
    ap.add_argument("--refresh", action="store_true",
                    help="ignore the cache and ask bmake again")
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--failed", action="store_true",
                    help="with --refresh, group the Makefiles bmake "
                         "would not read by directory")
    ap.add_argument("--names", action="store_true",
                    help="print every source named, one per line")
    ap.add_argument("query", nargs="*",
                    help="tree-relative paths to ask about")
    args = ap.parse_args()

    arches = args.arch or list(MACHINE_OF)
    total: set[str] = set()
    refused: set[str] = set()
    for a in arches:
        if args.refresh:
            named, failed = build(a, jobs=args.jobs)
            cache_path(a).write_text(json.dumps(sorted(named)))
            refused |= set(failed)
            print(f"{a:10s} {len(named):6d} sources named"
                  f"   ({len(failed)} Makefiles bmake would not read)")
        else:
            named = set(for_arch(a))
            print(f"{a:10s} {len(named):6d} sources named")
        total |= named
    print(f"{'union':10s} {len(total):6d}")

    if args.failed:
        if not args.refresh:
            print("\n(--failed needs --refresh: the cache keeps the "
                  "answers, not the refusals)")
        else:
            print(f"\nMakefiles bmake would not read: {len(refused)}")
            d = collections.Counter(x.rsplit("/", 1)[0] for x in refused)
            for k, v in sorted(d.items(), key=lambda kv: -kv[1]):
                print(f"  {v:4d}  {k}")

    if args.names:
        for n in sorted(total):
            print(n)
    for q in args.query:
        print(f"  {'named' if q in total else 'NOT NAMED':10s}  {q}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
