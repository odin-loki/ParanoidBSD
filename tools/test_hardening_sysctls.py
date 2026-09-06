#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A default behind an #ifdef is not a default until the config decides it.

Boot run 60 is the first PBSD system to be interrogated successfully, and
the step that read its `sysctl hardening` dump went red:

    FAIL hardening.kmalloc_zero: source says 1, kernel says 0

    17 knobs agree, 1 differ, 2 not present in the dump.

The kernel was right. kern_malloc.c declares the variable twice -

    #ifdef PAX_HARDEN_KMALLOC
    static int kmalloc_zero = PAX_FEATURE_SIMPLE_ENABLED;
    #else
    static int kmalloc_zero = PAX_FEATURE_SIMPLE_DISABLED;
    #endif

- and sys/conf/std.hardenedbsd:54 has `#options PAX_HARDEN_KMALLOC',
commented out. The checker did one re.search for an initialiser, took the
first, and so always reported the enabled branch of every guarded pair.

That is the worst kind of failing check: red because of the instrument,
against the only running system the project has, on the run that first
made the comparison possible at all. A reader who sees that twice stops
reading the step.

So the guards are resolved now, and this is what holds the resolution
honest. Everything here is a shape taken from the tree rather than an
invented one, because the point of the tool is to read this tree.
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hardening_sysctls as h  # noqa: E402

FAILURES: list[str] = []


def check(ok: bool, what: str) -> None:
    print(("  ok   " if ok else "  FAIL ") + what)
    if not ok:
        FAILURES.append(what)


# The kern_malloc.c shape, nested exactly as the file nests it.
KMALLOC = """
#ifdef PAX_HARDENING
#ifdef PAX_HARDEN_KMALLOC
static int kmalloc_zero = PAX_FEATURE_SIMPLE_ENABLED;
#else
static int kmalloc_zero = PAX_FEATURE_SIMPLE_DISABLED;
#endif
#endif
"""

# hbsd_pax_aslr.c: guarded by an option that std.hardenedbsd removes with
# nooptions, so no declaration survives and the sysctl is not in the kernel.
COMPAT32 = """
#ifdef COMPAT_FREEBSD32
static int pax_aslr_compat_status = PAX_FEATURE_OPTOUT;
#endif
"""

# The unguarded majority, which must keep working unchanged.
PLAIN = "static int pax_aslr_status = PAX_FEATURE_OPTOUT;\n"

# Both arms agreeing is not an ambiguity, whatever the config says.
AGREE = """
#ifdef SOMETHING_UNKNOWABLE
static int harden_tty = PAX_FEATURE_SIMPLE_ENABLED;
#else
static int harden_tty = PAX_FEATURE_SIMPLE_ENABLED;
#endif
"""


def main() -> int:
    print("the guard is read, not skipped")
    got = h.initialisers(KMALLOC, "kmalloc_zero")
    check(len(got) == 2, f"both declarations are found (got {len(got)})")
    if len(got) == 2:
        check(got[0] == ("PAX_FEATURE_SIMPLE_ENABLED",
                         ["defined(PAX_HARDENING)",
                          "defined(PAX_HARDEN_KMALLOC)"]),
              f"the #ifdef arm carries both guards ({got[0][1]})")
        check(got[1][1] == ["defined(PAX_HARDENING)",
                            "!(defined(PAX_HARDEN_KMALLOC))"],
              f"the #else arm carries the negation ({got[1][1]})")

    print("\nand the config decides which arm is compiled")
    on = {"PAX_HARDENING", "PAX_HARDEN_KMALLOC"}
    off = {"PAX_HARDENING"}
    check(h.default_of(KMALLOC, "kmalloc_zero", off) == ("0", "disabled"),
          "without PAX_HARDEN_KMALLOC the default is 0 - run 60's kernel")
    check(h.default_of(KMALLOC, "kmalloc_zero", on) == ("1", "enabled"),
          "with it, 1 - so the resolution is live, not a fixed second pick")
    check(h.default_of(KMALLOC, "kmalloc_zero", set())[0] == "?",
          "with PAX_HARDENING itself off, nothing is declared")
    check(h.default_of(KMALLOC, "kmalloc_zero", None)[0] == "?",
          "with no config at all, reported as conditional, not guessed")

    print("\nan unguarded default is untouched by any of this")
    check(h.default_of(PLAIN, "pax_aslr_status", None) == ("2", "opt-out"),
          "no config needed when there is only one declaration")
    check(h.default_of(PLAIN, "pax_aslr_status", set()) == ("2", "opt-out"),
          "and an empty config does not make it conditional")

    print("\nagreeing arms are not an ambiguity")
    check(h.default_of(AGREE, "harden_tty", None) == ("1", "enabled"),
          "two branches with the same value need no config to decide")

    print("\na knob no kernel compiles is not a knob that went missing")
    val, why = h.default_of(COMPAT32, "pax_aslr_compat_status",
                            {"PAX", "PAX_ASLR"})
    check(val == "?" and why.startswith("not compiled in"),
          f"COMPAT_FREEBSD32 off: {why[:40]}")
    check(h.default_of(COMPAT32, "pax_aslr_compat_status",
                       {"COMPAT_FREEBSD32"}) == ("2", "opt-out"),
          "COMPAT_FREEBSD32 on: the ordinary answer")

    print("\nthe #if evaluator says \"cannot tell\" rather than guessing")
    opts = {"A"}
    for expr, want in (("defined(A)", True),
                       ("defined(B)", False),
                       ("!defined(B)", True),
                       ("defined(A) && defined(B)", False),
                       ("defined(A) || defined(B)", True),
                       ("defined A", True),
                       ("PAX_ASLR_DELTA > 16", None),
                       ("__FreeBSD_version >= 1400000", None)):
        check(h.eval_cond(expr, opts) is want,
              f"{expr!r} -> {h.eval_cond(expr, opts)} (want {want})")

    print("\nconfig(8)'s own reading of a config file")
    with tempfile.TemporaryDirectory() as td:
        conf = Path(td) / "conf"
        conf.mkdir()
        (conf / "std.test").write_text(
            "options\t\tPAX\n"
            "options \tPAX_HARDENING\t# various\n"
            "#options\tPAX_HARDEN_KMALLOC\t# Harden malloc(9)\n"
            "nooptions\tCOMPAT_FREEBSD32\n")
        (conf / "BASE").write_text("options\tCOMPAT_FREEBSD32\n"
                                   "options\tSMP\n")
        (conf / "TEST").write_text('include BASE\ninclude "std.test"\n'
                                   "nooptions\tSMP\n")
        old = h.SYS
        h.SYS = Path(td)
        try:
            opts = h.config_options("TEST", "amd64")
        finally:
            h.SYS = old
    check("PAX" in opts and "PAX_HARDENING" in opts, "options are set")
    check("PAX_HARDEN_KMALLOC" not in opts,
          "a commented-out `#options' line sets nothing - the whole bug")
    check("COMPAT_FREEBSD32" not in opts,
          "a later nooptions removes what an include set")
    check("SMP" not in opts, "and nooptions after the includes still wins")
    check("__amd64__" in opts and "__LP64__" in opts,
          "the target's own predefined macros are in scope too")

    print("\nsys/conf/files decides whether a file is compiled at all")
    check(h.file_is_built("pax pax_aslr", {"PAX", "PAX_ASLR"}),
          "`optional pax pax_aslr' needs both, and has both")
    check(not h.file_is_built("pax pax_aslr", {"PAX"}),
          "and fails with only one")
    check(h.file_is_built("pax_aslr | pax_noexec", {"PAX_NOEXEC"}),
          "`a | b' needs either")
    check(not h.file_is_built("pax_aslr | pax_noexec", {"PAX"}),
          "and fails with neither")
    check(h.file_is_built("pax compile-with something pax_nonesuch",
                          {"PAX"}),
          "tokens after compile-with say how to build, not whether")
    check(h.file_is_built("standard-has-no-condition", None),
          "with no config, nothing is ruled out")

    print("\nagainst the real tree")
    if h.SYS.is_dir():
        files = h.built_files("amd64")
        check("hardenedbsd/hbsd_pax_aslr.c" in files,
              "hbsd_pax_aslr.c is a file some kernel compiles")
        check("hardenedbsd/hbsd_pax_SKEL.c" not in files,
              "hbsd_pax_SKEL.c is the copy-me template and is in no kernel")
        rows, skipped = h.collect(h.config_options("HARDENEDBSD", "amd64"),
                                  "amd64")
        by_path = {r[0]: r[1] for r in rows}
        check(by_path.get("hardening.kmalloc_zero") == "0",
              f"hardening.kmalloc_zero reads 0, as run 60's kernel did "
              f"(got {by_path.get('hardening.kmalloc_zero')})")
        check(by_path.get("hardening.pax.aslr.status") == "2",
              "hardening.pax.aslr.status still reads 2, opt-out")
        check(not any(p.startswith("hardening.pax.SKEL") for p in by_path),
              "and no knob comes from the template")
        check(any("hbsd_pax_SKEL.c" in s for s in skipped),
              "which is reported, not silently dropped")
    else:
        print("  skip  no hbsd/src/sys checkout here")

    print()
    if FAILURES:
        print(f"{len(FAILURES)} FAILED")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
