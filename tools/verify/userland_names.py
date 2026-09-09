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
import hashlib
import json
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE.parent.parent / "hbsd" / "src"
SHARE_MK = SRC / "share" / "mk"
OBJDIR = os.environ.get("PBSD_BMAKE_OBJDIR", "/tmp/pbsd_bmake_obj")

# What _bmake() pins the compiler to. In the cache key because a change
# here is a change to every answer below it.
CC_VARS = ("CC=clang", "CXX=clang++", "CPP=clang-cpp")


@functools.lru_cache(maxsize=None)
def compiler_key() -> str:
    """Which compiler answered - twelve hex digits, for the cache filename.

    The answers cached under /tmp are the BUILD's, and the build's answers
    depend on the compiler: share/mk's bsd.compiler.mk probes it, and every
    `.if ${COMPILER_TYPE} == "clang"' and `${COMPILER_FEATURES:M...}' in
    the tree turns on what it found. Asked as gcc, amd64's build names
    14,770 sources; asked as clang, 14,959. A cache written under one and
    read under the other is a reading that has quietly stopped matching -
    the failure this repository keeps finding in other people's code.

    So the compiler goes in the FILENAME. A different one misses the cache
    and rebuilds; it is never answered wrongly. Nothing to migrate and no
    stamp to parse - the stale files simply stop being opened.

    The probe is `clang --version' rather than bmake's COMPILER_TYPE and
    COMPILER_VERSION because those are DERIVED from this, by
    bsd.compiler.mk running the same binary, and this costs no bmake and
    cannot be reached before the cache path is needed. A compiler that is
    not there hashes as "not found", which is its own key: the one thing
    this must never do is give two different compilers the same one.

    REBUILD THE CACHES IN ONE PROCESS BEFORE A SWEEP. A cold cache makes
    for_arch() walk the whole tree with bmake, and every worker in a sweep
    does it independently - deleting them and starting a sweep took a libs
    shard from twenty minutes to a projected five hours at load average 45
    on four cores. `python3 tools/verify/userland_names.py --refresh' per
    architecture first, serially; installed_headers() is 82s more each.
    """
    parts = list(CC_VARS)
    for exe in ("clang", "clang++"):
        try:
            p = subprocess.run([exe, "--version"], capture_output=True,
                               text=True, timeout=60)
            first = p.stdout.split("\n")[0].strip()
            parts.append(f"{exe}: {first or 'no version line'}")
        except (OSError, subprocess.TimeoutExpired):
            parts.append(f"{exe}: not found")
    return hashlib.sha256("\x00".join(parts).encode()).hexdigest()[:12]

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
# krb5 is the MIT tree and kerberos5 is Heimdal, and Makefile.inc1:436
# picks between them:
#
#     .if ${MK_KERBEROS} != "no"
#     .if ${MK_MITKRB5} != "no"
#     SUBDIR+=krb5
#     .else
#     SUBDIR+=kerberos5
#     .endif
#
# MITKRB5 is __DEFAULT_YES, so the build descends into krb5 and never
# into kerberos5. This list had it the other way round: 54 kerberos5
# directories and no krb5 one, so <krb5.h> - which krb5/include installs
# from contrib/../crypto/krb5/src/include/krb5.h - was on no include
# path and usr.sbin/gssd was ERROR. Asking bmake in the Heimdal tree
# does not even work: src.libnames.mk:952 `.error's there, because
# _DP_krb5 is the MIT dependency set while that Makefile's LIBADD is the
# Heimdal one. kerberos5 stays, because a tree built with MK_MITKRB5=no
# is a legal configuration and its headers are then the right ones; it
# simply contributes nothing under the defaults.
SCOPES = ("lib", "libexec", "bin", "sbin", "usr.bin", "usr.sbin",
          "krb5", "kerberos5", "cddl", "rescue", "stand", "tests",
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
    # CC, because share/mk's bsd.compiler.mk PROBES the compiler and
    # every `.if ${COMPILER_TYPE} == "clang"' and `${COMPILER_FEATURES:M...}'
    # in the tree turns on the answer. Unset, it probes `cc' - gcc 13 on
    # this machine - while the instrument doing the analysing is clang
    # and PBSD's own build is clang. It changes what the build NAMES,
    # not only its flags: lib/libc/tests/gen puts three ATF tests inside
    # `.if ${COMPILER_FEATURES:Mblocks}', and asked as gcc bmake returns
    # fifty names without them.
    cmd = ["bmake", "-m", str(SHARE_MK), "CC=clang", "CXX=clang++",
           "CPP=clang-cpp"]
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
          ) -> tuple[set[str], list[str], dict[str, str]]:
    """(sources named, Makefiles bmake refused, source -> naming directory)

    The refusals are returned rather than counted so that a caller can
    see whether they matter: 94 of them sound alarming until they turn
    out to be libclang_rt, ofed and kerberos5, none of which any sweep
    scope has yet reached.

    The third value is what .PATH makes necessary. A source is compiled
    by the Makefile whose SRCS NAMES it, which is not always an ancestor
    of it: usr.bin/tip/tip/Makefile has

        CFLAGS+=-I${.CURDIR} ...
        .PATH:  ${.CURDIR}/../libacu
        SRCS=   acu.c acutab.c ... biz22.c biz31.c ...

    so usr.bin/tip/libacu/biz22.c is built in usr.bin/tip/tip with that
    -I, and the nearest ancestor holding a Makefile is usr.bin/tip,
    whose Makefile is `SUBDIR=tip' and has no flags at all. Ten of the
    first progs sweep's ERRORs were that one directory failing on
    `tip.h' file not found.

    ALL the directories that name it, not one. Two Makefiles naming the
    same source is common and the choice between them is not arbitrary:
    sbin/fsdb reaches into sbin/fsck_ffs and cddl/usr.bin/ctfconvert into
    cddl/contrib, and in both cases the source's OWN directory is one of
    the answers and is the right one. Keeping only the first resolved
    answer moved 265 sources, most of them to the wrong side of exactly
    that pair.
    """
    dirs = makefile_dirs(src)
    named: set[str] = set()
    failed: list[str] = []
    builder: dict[str, list[str]] = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        futs = {ex.submit(ask, d, arch, src): d for d in dirs}
        for f in concurrent.futures.as_completed(futs):
            d = futs[f]
            srcs, path = f.result()
            if not srcs and not path:
                failed.append(str(d.relative_to(src)))
                continue
            got = resolve(srcs, path, d, src)
            named |= got
            here = str(d.relative_to(src))
            for r in got:
                builder.setdefault(r, []).append(here)
    for r in builder:
        builder[r].sort()
    return named, sorted(failed), builder


# Flags a translation unit needs to PARSE, taken from a component's
# CFLAGS as well as from a file's own. Deliberately short: anything that
# only changes code generation belongs in the per-file carve-out, where
# -flto and -fsanitize can be excluded by name.
PARSE_AFFECTING = ("-fbracket-depth", "-fblocks")

# `-include <path>' is two words, so it survives neither the -I/-D/-U
# test nor the -f/-m one, and it is the strongest parse-affecting flag
# there is: it prepends a whole header. Thirty-two userland Makefiles
# use it, in both forms. usr.sbin/fstyp/Makefile:34 and :36 are
#
#     CFLAGS.zfs.c+= -include ${ZFSTOP}/include/os/freebsd/spl/sys/ccompile.h
#     CFLAGS.zfs.c+= -include ${SRCTOP}/sys/modules/zfs/zfs_config.h
#
# and without them zfs.c is fourteen errors starting inside libspl's
# own <string.h>.
INCLUDE_FLAG = "-include"


# The variables a directory can list programs in. Each may carry an
# SRCS.<prog>, and bsd.progs.mk / bsd.test.mk then read CFLAGS.<prog>.
_PROG_VARS = ("PROGS", "PROGS_CXX", "PLAIN_TESTS_C", "PLAIN_TESTS_CXX",
              "ATF_TESTS_C", "ATF_TESTS_CXX", "GTESTS")


@functools.lru_cache(maxsize=None)
def _srcs_of(d: Path, arch: str, src: Path, timeout: int,
             progs: tuple[str, ...]) -> tuple[tuple[str, frozenset[str]], ...]:
    """{program: the sources its SRCS.<prog> names}, given the programs.

    Cached on the DIRECTORY, not the file: SRCS.<prog> is a property of
    the Makefile, and asking it per file would be one bmake run for each
    of 1,862 progs units instead of one for each directory that lists
    programs at all.
    """
    if not progs:
        return ()
    srcs = _bmake(d, arch, [f"SRCS.{p}" for p in progs], src, timeout)
    if srcs is None:
        return ()
    return tuple((p, frozenset(line.split()))
                 for p, line in zip(progs, srcs))


@functools.lru_cache(maxsize=None)
def _progs_srcs(d: Path, arch: str, src: Path,
                timeout: int) -> tuple[tuple[str, frozenset[str]], ...]:
    """{program: the sources its SRCS.<prog> names} for one directory.

    The standalone form, which pays its own bmake run for the program
    lists. ask_cflags() does not use it: it folds those seven variables
    into the run it was already making. This is here for callers that
    want the answer on its own, and for the test that checks it.
    """
    got = _bmake(d, arch, list(_PROG_VARS), src, timeout)
    if got is None:
        return ()
    return _srcs_of(d, arch, src, timeout,
                    tuple(w for line in got for w in line.split()))


def _programs_naming(d: Path, arch: str, name: str, src: Path,
                     timeout: int) -> list[str]:
    """The programs in this directory whose SRCS.<prog> names `name'."""
    return [p for p, s in _progs_srcs(d, arch, src, timeout) if name in s]


def _take_flags(got: list[str], out: list[str], seen: set[str],
                own_from: int) -> None:
    """Append the -I, -D, -U (and per-file -f/-m) of each bmake line.

    `own_from' is the index at which the lines stop being the
    component's CFLAGS/CXXFLAGS and start being a single file's or a
    single program's own - which is the only place -f and -m are taken
    from, since -flto and -fsanitize live in the component's.
    """
    for i, line in enumerate(got):
        try:
            words = shlex.split(line)
        except ValueError:
            words = line.split()
        pending = False
        for w in words:
            if pending:
                # The path half of `-include <path>', emitted JOINED to
                # the flag. clang accepts `-include/path/x.h', and one
                # word is what every caller of this function expects:
                # includes.py drops a flag it has already seen, so two
                # `-include' words deduplicate to one and orphan the
                # second path, which clang then reads as a source file.
                # usr.sbin/fstyp has exactly two.
                pending = False
                w = INCLUDE_FLAG + w
            elif w == INCLUDE_FLAG:
                pending = True
                continue
            if w in seen or len(w) <= 2:
                continue
            if w.startswith(INCLUDE_FLAG) and len(w) > len(INCLUDE_FLAG):
                seen.add(w)
                out.append(w)
                continue
            # -f and -m only from the file's own flags, and never the
            # two the analyser cannot accept.
            #
            # PARSE_AFFECTING is the exception, and it comes from the
            # same reasoning the per-file carve-out already uses:
            # without them the file does not PARSE, so dropping them
            # does not produce a differently-optimised compile, it
            # produces an ERROR. usr.sbin/wlanstat/Makefile:10 is
            #
            #     CFLAGS.clang+= -fbracket-depth=512 -Wno-cast-align
            #
            # for a file whose ~300 `#define S_X AFTER(S_PREV)' chain
            # expands to that many nested parentheses; without it,
            # `fatal error: bracket nesting level exceeded maximum of
            # 256'. -fblocks is the same shape one extension over, and
            # appears in component CFLAGS as well as per-file ones.
            if (w[:2] in ("-I", "-D", "-U")
                    or w.startswith(PARSE_AFFECTING)
                    or (i >= own_from and w[:2] in ("-f", "-m")
                        and not w.startswith(("-fsanitize", "-flto")))):
                seen.add(w)
                out.append(w)


def ask_cflags(d: Path, arch: str, src: Path = SRC, timeout: int = 40,
               name: str = "") -> list[str]:
    """The -I, -D and -U this directory's build really passes.

    includes.py reads a component's Makefile.inc chain by walking from
    the source's directory up to the component root, which is right for
    lib/libc and wrong for anything that reaches sideways. libexec/atrun
    is three lines of Makefile and one of them is

        MAINSRC=${SRCTOP}/usr.bin/at
        .include "${MAINSRC}/Makefile.inc"

    which is not an ancestor of anything. DAEMON_GID, DAEMON_UID,
    ATJOB_DIR, PERM_PATH and five more -D live in that file, and without
    them atrun.c does not compile - `use of undeclared identifier
    DAEMON_GID'. No walk up the tree finds it; bmake does, because
    following .include is what bmake is.

    So the flags are read out of the build rather than re-derived from
    it. Only -I, -D and -U are taken: the rest of a real CFLAGS is
    HardenedBSD's hardening (-fsanitize=cfi, -mretpoline, -flto) and
    the tree's warning policy, none of which the analyser wants and
    some of which it cannot accept. shlex does the splitting, because
    bmake escapes for sh and a -D can carry a quoted string with a
    space in it:

        -DATJOB_DIR=\"/var/at/jobs/\"
    """
    want = ["CFLAGS", "CXXFLAGS"]
    if name:
        # A file's OWN flags, which the component's CFLAGS do not carry.
        # The tree spells the variable both ways - with the suffix and
        # without - so both are asked:
        #
        #   lib/libc/gen/Makefile.inc:185   CFLAGS.dlfcn.c= ${RTLD_HDRS}
        #   lib/libc/tests/stdtime/Makefile:6
        #       CFLAGS.detect_tz_changes_test+= -I${SRCTOP}/contrib/tzcode
        #   lib/libc/tests/gen/Makefile:120 CFLAGS.${t}.c+= -fblocks
        #
        # Without the first, rtld.h cannot find rtld_machdep.h, which
        # lives in libexec/rtld-elf/<cpuarch>/. Without the last the
        # file does not PARSE - blocks are a language extension, not an
        # include path - which is why -f is passed through here and not
        # in the component flags, where -flto and -fsanitize live.
        stem = name.rsplit(".", 1)[0]
        want += [f"CFLAGS.{name}", f"CFLAGS.{stem}",
                 f"CXXFLAGS.{name}", f"CXXFLAGS.{stem}"]
        # And the flags of the PROGRAM this file belongs to, which
        # bsd.progs.mk and bsd.test.mk spell CFLAGS.<prog> rather than
        # CFLAGS.<file>. sbin/dhclient/tests/Makefile is the case:
        #
        #     PLAIN_TESTS_C=            option-domain-search_test
        #     SRCS.option-domain-search_test= alloc.c ... fake.c \
        #                                     option-domain-search.c
        #     CFLAGS.option-domain-search_test+= -I${.CURDIR:H}
        #
        # fake.c has no CFLAGS.fake.c of its own; the -I that lets it
        # find <dhcpd.h> is on the test program's name.
        #
        # Which programs those are needs a bmake run of its own - and
        # a run per directory is 620 of them over bin sbin usr.bin
        # usr.sbin, which measured as a third of the sweep's wall time
        # for an answer that is empty in almost every directory. So the
        # seven variables ride along in the run this function was
        # already making: more variables on one bmake invocation cost
        # nothing, and only a directory that actually lists programs
        # then pays for the SRCS.<prog> run that follows.
        prog_from = len(want)
        want += list(_PROG_VARS)
    got = _bmake(d, arch, want, src, timeout)
    if got is None:
        return []

    progs: tuple[str, ...] = ()
    if name:
        progs = tuple(w for line in got[prog_from:] for w in line.split())
        got = got[:prog_from]

    out: list[str] = []
    seen: set[str] = set()
    _take_flags(got, out, seen, own_from=2)

    if progs:
        mine = [pr for pr, s in _srcs_of(d, arch, src, timeout, progs)
                if name in s]
        if mine:
            more = _bmake(d, arch,
                          [f"{v}.{pr}" for pr in mine
                           for v in ("CFLAGS", "CXXFLAGS")],
                          src, timeout)
            if more:
                _take_flags(more, out, seen, own_from=0)
    return out


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
        f"pbsd_userland_incs_{arch}_{compiler_key()}.json"


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


def module_dirs(src: Path = SRC) -> list[Path]:
    """Every sys/modules directory with a Makefile."""
    top = src / "sys" / "modules"
    out = []
    if top.is_dir():
        for dirpath, dirnames, filenames in os.walk(top):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            if "Makefile" in filenames:
                out.append(Path(dirpath))
    return sorted(out)


def ask_module(d: Path, arch: str, src: Path = SRC, timeout: int = 40
               ) -> tuple[dict[str, tuple[str, ...]], list[str], set[str]]:
    """({source: its per-file CFLAGS}, .PATH) for one kernel module.

    The kernel half of the same problem userland had. includes.py hand-
    parses `SRCS' out of sys/modules/*/Makefile, and a module that names
    its sources any other way is invisible to it. sys/modules/blake2 is
    the case that showed it: its ten SIMD implementations are reached
    through

        SRCS_IN += blake2b-avx.c
        OBJS    += ${SRCS_IN:S/.c/.o/g}
        .for src in ${SRCS_IN}
        ${src:S/.c/.o/}: ${src}
                ${CC} -c ${CFLAGS:N-nostdinc} ${CFLAGS.${src}} ...

    so no SRCS line names them and the hand parser reported all ten as
    "nothing in the build names it". They also do not compile without
    the flags on the line below - CFLAGS.blake2b-avx.c is
    `-DSUFFIX=_avx -msse2 -mssse3 -msse4.1 -mavx' - which is why they
    were ERROR as well as unlisted.

    bmake reads both. OBJS is asked alongside SRCS and each .o is mapped
    back to its source through .PATH; then CFLAGS.<basename> is asked
    for every source found, in one further invocation.

    Returns the sources with the flags each one's own CFLAGS.<file>
    adds - not the module's whole CFLAGS, which is kern.pre.mk's job and
    already in includes.kernel_flag_index().
    """
    got = _bmake(d, arch, ["SRCS", "OBJS", ".PATH", ".ALLTARGETS"],
                 src, timeout)
    if got is None:
        return {}, [], set()
    srcs, objs, path, targets = (g.split() for g in got)
    names = [s for s in srcs if s.endswith(SUFFIXES) and "$" not in s]
    # Which of them came from OBJS, and so are built by a rule of the
    # module's own rather than by the ordinary one. That distinction is
    # the whole reason to separate them: a rule's flags belong to its
    # target and not to everything the module names. sys/modules/linux
    # has `OBJS= linux_vdso.so' and a locore rule carrying -DLOCORE, and
    # giving that -D to all of sys/compat/linux told every header the
    # file was assembly - twenty-six regressions in one sweep.
    # kmod.mk appends every SRCS-derived object to OBJS as well, so
    # "came from OBJS" is not the question - "is in OBJS and NOT in
    # SRCS" is. That is exactly the .for-rule set: blake2's ten SIMD
    # objects are in OBJS alone, and sys/modules/linux's linux_file.o is
    # in both. Using the first question gave sys/compat/linux the
    # locore rule's -DLOCORE and told every header those files were
    # assembly - twenty-six regressions in one sweep.
    have = set(srcs)
    from_objs: set[str] = set()
    # ...and .ALLTARGETS, for a rule whose target is in NEITHER.
    #
    # sys/modules/vmm/Makefile has
    #
    #   CLEANFILES+=  vmm_nvhe_exception.o vmm_nvhe.o
    #   vmm_nvhe.o: vmm_nvhe.c vmm_hyp.c
    #           ${CC} -c ${NOSAN_CFLAGS:...} ${.IMPSRC} -o ${.TARGET} -fpie
    #   vmm_hyp_blob.elf.full: vmm_nvhe_exception.o vmm_nvhe.o
    #
    # so vmm_nvhe.o IS built - the hypervisor blob the module links
    # depends on it - and it appears nowhere but CLEANFILES and its own
    # rule. SRCS and OBJS cannot see it; .ALLTARGETS is bmake's list of
    # every target it has a rule for, and it names both.
    #
    # The test stays "ends in .o and resolves through .PATH to a file
    # that exists", because CLEANFILES also lists things built from no
    # source in this tree - vmm_hyp_blob.bin among them - and .ALLTARGETS
    # carries the module's phony targets too.
    for o in list(objs) + [t for t in targets if t not in set(objs)]:
        if o.endswith(".o") and "$" not in o and o not in have:
            c = o[:-2] + ".c"
            if c not in have and c not in from_objs:
                names.append(c)
                from_objs.add(c)
    dirs = [str(d)] + [x for x in path if x != "."]
    found: dict[str, str] = {}
    obj_rels: set[str] = set()
    for n in names:
        for pdir in dirs:
            cand = os.path.join(pdir, n)
            if os.path.isfile(cand):
                try:
                    r = str(Path(cand).resolve().relative_to(src))
                    found[r] = n
                    if n in from_objs:
                        obj_rels.add(r)
                except ValueError:
                    pass
                break
    if not found:
        return {}, path, set()
    rel = sorted(found)
    per = _bmake(d, arch, [f"CFLAGS.{found[r]}" for r in rel], src, timeout)
    out: dict[str, tuple[str, ...]] = {}
    for i, r in enumerate(rel):
        line = per[i] if per and i < len(per) else ""
        try:
            words = shlex.split(line)
        except ValueError:
            words = line.split()
        out[r] = tuple(words)
    return out, path, obj_rels


def cache_path(arch: str) -> Path:
    return Path(os.environ.get("PBSD_CACHE", "/tmp")) / \
        f"pbsd_userland_names_{arch}_{compiler_key()}.json"


def builder_cache_path(arch: str) -> Path:
    return Path(os.environ.get("PBSD_CACHE", "/tmp")) / \
        f"pbsd_userland_builder_{arch}_{compiler_key()}.json"


@functools.lru_cache(maxsize=None)
def builders(arch: str) -> dict[str, list[str]]:
    """source (tree-relative) -> every directory whose SRCS names it.

    Written by the same walk that fills the names cache, so asking for
    it does not run bmake a second time; an old cache without the file
    just rebuilds both.
    """
    p = builder_cache_path(arch)
    if p.is_file():
        try:
            return json.loads(p.read_text())
        except (ValueError, OSError):
            pass
    for_arch(arch, refresh=True)
    try:
        return json.loads(p.read_text())
    except (ValueError, OSError):
        return {}


@functools.lru_cache(maxsize=None)
def for_arch(arch: str, refresh: bool = False) -> frozenset[str]:
    p = cache_path(arch)
    if not refresh and p.is_file():
        try:
            return frozenset(json.loads(p.read_text()))
        except (ValueError, OSError):
            pass
    named, _, builder = build(arch)
    try:
        p.write_text(json.dumps(sorted(named)))
        builder_cache_path(arch).write_text(json.dumps(builder, sort_keys=True))
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
            named, failed, builder = build(a, jobs=args.jobs)
            cache_path(a).write_text(json.dumps(sorted(named)))
            builder_cache_path(a).write_text(
                json.dumps(builder, sort_keys=True))
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
