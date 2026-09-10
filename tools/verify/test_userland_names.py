#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The userland authority, checked against facts it must get right.

An index that answers "no" to everything looks exactly like a tree the
build names nothing in, which is how sweep_report came to report 57 of
57 unlisted ERRORs as "nothing in the build names it" and mean nothing
by it. Each case below is one way the index can quietly go empty:

  * the plain SRCS reading - lib/libc/gen/getcwd.c;
  * the .PATH reading, which is what makes a lib/libc source that lives
    under contrib resolve at all;
  * the PROGS reading, without which every tests/ directory in the tree
    names nothing - fifty programs in lib/libc/tests/gen alone;
  * the per-architecture reading: lib/libc/aarch64 is named when
    MACHINE_ARCH is aarch64 and by nothing else, and an index built for
    one architecture and used for all six would say so for amd64 too;
  * the scope overlap: lib/libc's build names three sources under sys/,
    so the two authorities are unioned and not selected between;
  * and the INCS reading, which is the same question asked of headers:
    what each Makefile installs into /usr/include, and where.

Each assertion is recomputed from the tree, so a file that moves takes
the assertion with it rather than leaving a stale expectation behind.
"""

from __future__ import annotations

import shutil
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import userland_names as u  # noqa: E402

FAIL = 0


def check(cond: bool, what: str) -> None:
    global FAIL
    print(f"  {'ok  ' if cond else 'FAIL'}  {what}")
    if not cond:
        FAIL += 1


def one_dir(rel: str, arch: str) -> set[str]:
    d = u.SRC / rel
    srcs, path = u.ask(d, arch)
    return u.resolve(srcs, path, d)


def main() -> int:
    if not shutil.which("bmake"):
        print("bmake is not installed; the userland authority cannot be "
              "asked. Skipping.")
        return 0
    if not u.SHARE_MK.is_dir():
        print(f"no {u.SHARE_MK}; skipping.")
        return 0

    print("one directory at a time, so a failure names the reading "
          "that broke:")

    # SRCS, read straight. lib/libc/gen/getcwd.c is in SRCS via
    # lib/libc/gen/Makefile.inc, and reaches lib/libc through .PATH.
    libc = one_dir("lib/libc", "amd64")
    check("lib/libc/gen/getcwd.c" in libc,
          "SRCS: lib/libc names gen/getcwd.c")
    check(len(libc) > 500,
          f"SRCS: lib/libc names hundreds of sources ({len(libc)})")

    # .PATH into another tree entirely: libc's gdtoa comes from contrib.
    check(any(p.startswith("contrib/") for p in libc),
          ".PATH: some of what lib/libc names lives under contrib/")

    # PROGS, without which a tests directory names nothing.
    tests = one_dir("lib/libc/tests/gen", "amd64")
    check(len(tests) > 20,
          f"PROGS: lib/libc/tests/gen names its programs ({len(tests)})")

    # Per architecture. lib/libc/aarch64/gen/getcontextx.c is named when
    # MACHINE_ARCH is aarch64, and by no other architecture's build.
    a64 = one_dir("lib/libc", "aarch64")
    mine = "lib/libc/aarch64/gen/getcontextx.c"
    check(mine in a64, f"per-arch: aarch64 names {Path(mine).name}")
    check(mine not in libc, "per-arch: amd64 does not name it")

    # And a source no architecture's build names: the db test drivers,
    # which have no Makefile at all - they are run by hand.
    scratch = "lib/libc/db/test/hash.tests/tdel.c"
    check((u.SRC / scratch).is_file(), f"the tree still has {scratch}")
    check(scratch not in libc, "scratch: nothing in lib/libc names tdel.c")

    # The trees are not disjoint. lib/libc's build reaches into the
    # kernel's for three sources, which is why sweep_report asks both
    # authorities rather than picking one by the path's first component
    # - routing sys/ to the kernel would answer for these three with a
    # list that has never heard of them.
    from_sys = sorted(p for p in libc if p.startswith("sys/"))
    check(len(from_sys) >= 3,
          f"scope: lib/libc's build names kernel sources too ({from_sys})")

    # What each Makefile installs into /usr/include, which is how a
    # program reaches <devstat.h> and <security/pam_appl.h> - neither
    # of which is anywhere near the program that includes it.
    incs = u.ask_incs(u.SRC / "lib/libdevstat", "amd64")
    check(incs.get("devstat.h", "").endswith("lib/libdevstat/devstat.h"),
          "INCS: lib/libdevstat installs devstat.h")

    # A group that is not INCS, and a subdirectory of ${INCLUDEDIR}.
    # lib/libpam/libpam lists `security/pam_appl.h' and installs it into
    # ${INCLUDEDIR}/security, so the installed path is
    # security/pam_appl.h - not security/security/pam_appl.h, which is
    # what joining the two without a basename gives.
    pam = u.ask_incs(u.SRC / "lib/libpam/libpam", "amd64")
    check("security/pam_appl.h" in pam,
          f"INCSDIR: pam_appl.h installs one level deep "
          f"({sorted(pam)[:1]})")

    # And the .PATH reading again, from the other side: that header is
    # not in lib/libpam/libpam at all, it is in contrib/openpam.
    check(pam.get("security/pam_appl.h", "").startswith(
              str(u.SRC / "contrib/openpam")),
          "INCS: and it comes from contrib/openpam through .PATH")

    # And the kernel side of the same reading. sys/modules/blake2 names
    # its ten SIMD sources through SRCS_IN and OBJS, not SRCS, and none
    # of them compiles without the per-file CFLAGS on the next line.
    # A reader that sees only SRCS reports all ten as built by nothing.
    mod, _, objs = u.ask_module(u.SRC / "sys/modules/blake2", "amd64")
    avx = "sys/crypto/blake2/blake2b-avx.c"
    check(avx in mod, f"OBJS: sys/modules/blake2 names {Path(avx).name}")
    check("-mavx" in mod.get(avx, ()),
          f"CFLAGS.<file>: ...with its own -mavx ({mod.get(avx)})")
    check(avx in objs,
          "...and it is known to come from OBJS, so the rule's own "
          "flags reach it and nothing else")

    # The compiler the probe describes. share/mk's bsd.compiler.mk runs
    # `cc' and every `.if ${COMPILER_TYPE} == "clang"' and
    # `${COMPILER_FEATURES:M...}' in the tree turns on the answer - and
    # unset, `cc' is gcc here while the analyser is clang. It changes
    # what the build NAMES, not only its flags.
    gen = u.SRC / "lib/libc/tests/gen"
    names, _ = u.ask(gen, "amd64")
    check("fts_blocks_test.c" in names,
          "COMPILER_FEATURES:Mblocks names lib/libc/tests/gen's blocks "
          "tests")

    # A file's OWN CFLAGS.<file>, which a component's CFLAGS do not
    # carry. Three shapes, all in libc.
    dl = u.ask_cflags(u.SRC / "lib/libc", "amd64", name="dlfcn.c")
    check(any(f.endswith("libexec/rtld-elf") for f in dl),
          "CFLAGS.dlfcn.c is ${RTLD_HDRS}, and rtld.h needs it")
    check(not any(f.endswith("libexec/rtld-elf")
                  for f in u.ask_cflags(u.SRC / "lib/libc", "amd64")),
          "...and the component's own CFLAGS do not carry it")
    bl = u.ask_cflags(gen, "amd64", name="fts_blocks_test.c")
    check("-fblocks" in bl,
          "CFLAGS.${t}.c is -fblocks, and the file will not PARSE "
          "without it")
    check("-fblocks" not in u.ask_cflags(gen, "amd64"),
          "...and this directory's component CFLAGS do not carry it")
    wl = u.ask_cflags(u.SRC / "usr.sbin/wlanstat", "amd64", name="wlanstat.c")
    check("-fbracket-depth=512" in wl,
          "a PARSE_AFFECTING flag is taken from the COMPONENT's CFLAGS: "
          "wlanstat/Makefile:10 is CFLAGS.clang+= -fbracket-depth=512, and "
          "the ~300-deep AFTER() chain does not parse at clang's default 256")
    check(not any(f.startswith(("-flto", "-fsanitize")) for f in wl),
          "...and the ones the analyser cannot accept are still excluded")
    check(not any(f == "-Wno-cast-align" for f in wl),
          "...and a -W on the same line is not a parse-affecting flag")
    zf = u.ask_cflags(u.SRC / "usr.sbin/fstyp", "amd64", name="zfs.c")
    _inc = [f for f in zf if f.startswith("-include")]
    check(len(_inc) == 2,
          "BOTH -include reach the compiler: fstyp/Makefile:34 and :36 "
          f"are two of them, and zfs.c does not compile without either "
          f"(got {len(_inc)})")
    check(all(len(f) > len("-include") for f in _inc),
          "...joined to their paths, because a bare `-include' repeats "
          "and includes.py drops a flag it has seen, orphaning the path "
          "into a second source file")
    dh = u.ask_cflags(u.SRC / "sbin/dhclient/tests", "amd64", name="fake.c")
    check(any(f.endswith("sbin/dhclient") for f in dh),
          "a file gets its PROGRAM's flags: fake.c is in "
          "SRCS.option-domain-search_test, and the -I that finds "
          "<dhcpd.h> is CFLAGS.option-domain-search_test")
    check(not any(f.endswith("sbin/dhclient") for f in
                  u.ask_cflags(u.SRC / "sbin/dhclient/tests", "amd64")),
          "...and not from the directory's own CFLAGS, which lack it")

    _scopes = set(u.SCOPES)
    check("krb5" in _scopes,
          "Makefile.inc1:438 descends into krb5 when MK_MITKRB5 != no, "
          "which is the default; without this <krb5.h> is nowhere")
    _dirs = u.makefile_dirs(u.SRC)
    check(any(str(d).endswith("/krb5/include") for d in _dirs),
          "...and the walk reaches krb5/include, which installs it")

    tz = u.ask_cflags(u.SRC / "lib/libc/tests/stdtime", "amd64",
                      name="detect_tz_changes_test.c")
    check(any("contrib/tzcode" in f for f in tz),
          "CFLAGS.<file> is spelled without the suffix too "
          "(detect_tz_changes_test)")

    # OBJS, and the two directories it must NOT read.
    #
    # lib/csu names every object it builds in OBJS and writes no SRCS
    # line at all, so crtbegin.c and crtend.c -- which every C program
    # on the system starts in -- were named by nothing and analysed by
    # nothing. They live in lib/csu/common and are compiled from the
    # ARCH directory, whose -I${.CURDIR} is where that architecture's
    # crt.h is, so the builder directory matters as much as the name.
    csu = u.SRC / "lib/csu/amd64"
    s, pth = u.ask(csu, "amd64")
    got = u.resolve(s, pth, csu)
    check("lib/csu/common/crtbegin.c" in got,
          "OBJS names crtbegin.c through .PATH (lib/csu/amd64)")
    check("lib/csu/common/crtend.c" in got,
          "...and crtend.c with it")
    check(not any(g.endswith("crtbeginS.c") or g.endswith("Scrt1.c")
                  for g in got),
          "...and an .o with no source of its own falls away "
          "(crtbeginS.o, Scrt1.o)")

    # The other side. These two are 4.4BSD-era standalone Makefiles that
    # `.include' nothing, name paths through an undefined ${PORTDIR} and
    # are run by hand -- but they DO set PROG and OBJS, so reading OBJS
    # without asking whether the directory is part of this build called
    # their five sources built and broke two NOT_NAMED prefixes that are
    # true. bmake's own .MAKE.MAKEFILES is the test: a directory the
    # build enters reaches bsd.lib.mk or bsd.prog.mk, these reach only
    # sys.mk's unconditional fragments.
    for d in ("lib/libc/db/test", "lib/libc/regex/grot"):
        s, pth = u.ask(u.SRC / d, "amd64")
        check(not u.resolve(s, pth, u.SRC / d),
              f"{d} is not part of this build, so its OBJS is not read")
    check(u.BUILD_MK.search("/x/share/mk/bsd.lib.mk") is not None and
          u.BUILD_MK.search("/x/share/mk/bsd.mkopt.mk") is None,
          "the build-membership test names bsd.lib.mk and not the "
          "fragments sys.mk gives every directory")

    # How MANY bmake runs the answer costs, not just what it is. The
    # program lists used to be a run of their own, per directory: 626 of
    # them over bin sbin usr.bin usr.sbin, for an answer that is empty in
    # almost every one. They now ride along in the run ask_cflags was
    # already making, and only a directory that really lists programs
    # pays for the SRCS.<prog> run that follows. Counted rather than
    # asserted, because nothing else would notice the extra run coming
    # back.
    real_bmake = u._bmake
    runs: list[int] = []

    def counting(*a, **k):
        runs.append(1)
        return real_bmake(*a, **k)

    u._bmake = counting
    try:
        u._srcs_of.cache_clear()
        u._progs_srcs.cache_clear()
        runs.clear()
        u.ask_cflags(u.SRC / "usr.bin/killall", "amd64", name="killall.c")
        plain = len(runs)

        u._srcs_of.cache_clear()
        runs.clear()
        withprog = u.ask_cflags(u.SRC / "sbin/dhclient/tests", "amd64",
                                name="fake.c")
        prog = len(runs)
    finally:
        u._bmake = real_bmake

    check(plain == 1,
          "a directory with no programs costs ONE bmake run, not two "
          f"(got {plain})")
    check(prog == 3,
          "a directory with programs pays for SRCS.<prog> and the "
          f"program's own flags, and no more (got {prog})")
    check(any(f.endswith("sbin/dhclient") for f in withprog),
          "...and still returns the program's -I after the fold")

    print(f"\n{'FAILED' if FAIL else 'all checks passed'}"
          f"{f' ({FAIL})' if FAIL else ''}")
    return 1 if FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
