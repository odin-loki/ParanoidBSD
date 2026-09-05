#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Classify assembly as irreducible or eliminable.

"No assembly, everything ISO C++23" cannot be met in full, and it is worth
being precise about why rather than discovering it late. ISO C++23 has no
construct that sets a stack pointer, returns via IRET, writes CR3, or names a
register - the standard deliberately says nothing about registers, privilege
levels, or what runs before main. Note that __asm__ and compiler intrinsics
are not ISO C++ either, so "no .S files but inline asm" is a different and
much weaker claim.

What is reachable is a small audited nucleus with everything else in ISO
C++23. This sorts the tree so that goal has a number attached:

  irreducible  boot entry, trap and interrupt vectors, context switch,
               privileged instructions, signal trampolines
  eliminable   atomics and barriers  -> std::atomic, atomic_ref, fences
               bit twiddling         -> <bit>: popcount, countl_zero,
                                        byteswap, bit_cast
               string and math asm   -> written for speed, C fallbacks exist
  test-only    assembly that is the subject of a test rather than part of
               the system - rewriting it would delete what the test tests
  not assembly linker scripts named .lds.s, counted by the suffix and by
               nothing else

Usage:  python3 tools/asm_inventory.py [--list CATEGORY]
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "hbsd" / "src"

# Ordered: the first pattern that matches a path wins.
IRREDUCIBLE = [
    ("boot entry",        re.compile(r"(locore|mpboot|start|btx|boot1|boot2|xen-locore)", re.I)),
    ("trap/interrupt",    re.compile(r"(exception|_vector|trap|intr)", re.I)),
    ("context switch",    re.compile(r"(cpu_switch|swtch|setjmp|longjmp|context)", re.I)),
    ("signal trampoline", re.compile(r"(sigtramp|sigcode)", re.I)),
    ("privileged/support",re.compile(r"(support|cpufunc|machdep|msr|apic|efirt)", re.I)),
]
# Everything the ordered lists above leave over, sorted by hand. These run
# after them and only ever see what is still unclassified, so adding one
# cannot change a verdict that was already reached.
IRREDUCIBLE_EXTRA = [
    # crt1 sets up the stack and calls main; there is no ISO C++23 way to
    # write what runs before main.
    ("C runtime startup",   re.compile(r"^lib/csu/[^/]+/(crt1_s|crti|crtn)\.S$")),
    # Reads the syscall return convention - amd64 signals failure in the
    # carry flag, which C++ cannot see.
    ("syscall error path",  re.compile(r"^lib/libsys/[^/]+/(cerror|_umtx_op_err)\.S$")),
    # vfork must not touch the stack between syscall and return; the rfork
    # variants return onto a different stack entirely.
    ("fork/stack switch",   re.compile(r"(^|/)(vfork|rfork_thread|pdrfork_thread)\.S$")),
    ("ACPI resume trampoline", re.compile(r"acpi_wakecode")),
    ("BIOS/real mode",      re.compile(r"(bioscall|vm86bios|smapi_bios)")),
    # Fault handling by instruction address: the fixup table names the
    # faulting instruction, so the instruction has to be spelled out.
    ("fault-tolerant copy", re.compile(r"(copyinout|copyout_fast)")),
    ("hypervisor entry",    re.compile(r"(hyp_stub|vmm_call|vmm_hyp|smccc)")),
    ("MMIO accessors",      re.compile(r"bus_space_asm")),
    # .incbin. C++26 gets #embed; C++23 does not have it.
    ("embedded binary",     re.compile(r"(firmw|embedfs|fdt_static_dtb|vdso_inc|vdso_wrap)")),
    # ticks = ticksl + offset: a symbol at an offset from another symbol.
    ("symbol aliasing",     re.compile(r"subr_ticks")),
    # Restoring arm, powerpc, riscv and i386 brought their nuclei with them.
    # Same categories, different instruction sets - which is the argument for
    # deleting the eliminable ones: every architecture pays for those twice.
    ("C runtime startup",   re.compile(r"(crtsavres|powerpc/gen/eabi)\.S$")),
    ("syscall error path",  re.compile(r"^lib/libsys/[^/]+/syscall\.S$")),
    ("fault-tolerant copy", re.compile(r"(copystr|fusu)\.S$")),
    # Reads the TLS register; moves the stack pointer.
    ("privileged/support",  re.compile(r"(__aeabi_read_tp|alloca|setcpsr|setstack|cpu_asm-v6|cpu_subr64)\.S$")),
    ("hypervisor entry",    re.compile(r"(hypervisor-stub|ti_smc|vmm_switch|mambocall|ofwcall\d*|opalcall|hvcall)\.S$")),
    ("boot entry",          re.compile(r"mptramp\.S$")),
]

TEST_ONLY = re.compile(r"^(tests/|tools/regression/|tools/test/)")

# .lds.s is a linker script run through cpp. It is not assembly and never was.
NOT_ASSEMBLY = re.compile(r"\.lds\.s$")

ELIMINABLE_EXTRA = [
    # .section .note.* and nothing else - a struct with an attribute does it.
    ("ELF notes",       re.compile(r"^lib/csu/common/(crtbrand|feature_note|ignore_init_note)\.S$")),
    ("string/memory",   re.compile(r"^lib/libc/[^/]+/string/|^sys/libkern/[^/]+/memclr\.S$|^sys/arm64/arm64/strncmp\.S$")),
    ("math",            re.compile(r"^lib/libc/[^/]+/gen/fabs\.S$")),
    ("integer division",re.compile(r"^lib/libc/[^/]+/stdlib/(div|ldiv|lldiv)\.S$|^sys/libkern/[^/]+/(divsi3|ldivmod)\.S$")),
    ("checksum",        re.compile(r"(crc32|in_cksum_\w+\.S$)")),
    # ARM EABI soft-float and division helpers. compiler-rt ships C for all
    # of these; they are here for speed on a target that has no VFP.
    ("integer division",re.compile(r"^lib/libc/arm/(aeabi/aeabi_(int_div|.*div)|gen/divsi3)\.S$")),
    ("math",            re.compile(r"^lib/libc/arm/aeabi/aeabi_(asm|vfp)_(double|float)\.S$")),
    ("crypto/SIMD",     re.compile(r"rmd160")),
    ("ELF notes",       re.compile(r"elf_note\.S$")),
]

ELIMINABLE = [
    ("atomics/barriers",  re.compile(r"(atomic|fence|barrier|lock)", re.I)),
    ("bit ops",           re.compile(r"(bswap|ffs|fls|popcnt|bit)", re.I)),
    ("string/memory",     re.compile(r"(mem(cpy|set|move|cmp)|str(cpy|len|cmp|chr)|bcopy|bzero)", re.I)),
    ("math",              re.compile(r"(^|/)(e_|s_|k_)|fpu|npx|sqrt|exp|log|sin|cos|tan", re.I)),
    ("crypto/SIMD",       re.compile(r"(aes|sha|gcm|chacha|poly1305|blake|md5|des|rc4|sse|avx|simd)", re.I)),
]


# Third-party trees. LLVM's test suite and arm-optimized-routines are
# upstream's assembly, not PBSD's to rewrite, and counting them buries the
# number that matters.
# For --by-arch. A path under sys/arm64 or lib/libc/amd64 belongs to that
# architecture; anything with no such segment is shared by all of them.
ARCH_DIR = re.compile(r"(^|/)(aarch64|arm64|armv[67]|arm|amd64|i386|x86|"
                      r"powerpc64le|powerpc64|powerpcspe|powerpc|"
                      r"riscv64|riscv|mips64|mips)(/|$)")
ARCH_ALIAS = {"aarch64": "arm64", "armv6": "arm", "armv7": "arm",
              "x86": "amd64/i386", "powerpc64": "powerpc",
              "powerpc64le": "powerpc", "powerpcspe": "powerpc",
              "riscv64": "riscv", "mips64": "mips"}

VENDORED = re.compile(r"^(contrib/|sys/contrib/|crypto/|sys/crypto/|"
                      r"secure/|cddl/|sys/cddl/|sys/dev/[^/]+/firmware)")


def classify(rel: str) -> tuple[str, str]:
    if NOT_ASSEMBLY.search(rel):
        return "not assembly", "linker script"
    if VENDORED.match(rel):
        return "vendored", "third-party"
    if TEST_ONLY.match(rel):
        return "test-only", "assembly under test"
    for label, pat in IRREDUCIBLE:
        if pat.search(rel):
            return "irreducible", label
    for label, pat in ELIMINABLE:
        if pat.search(rel):
            return "eliminable", label
    for label, pat in IRREDUCIBLE_EXTRA:
        if pat.search(rel):
            return "irreducible", label
    for label, pat in ELIMINABLE_EXTRA:
        if pat.search(rel):
            return "eliminable", label
    return "unclassified", "needs review"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--list", metavar="CATEGORY",
                    help="print the paths in one category")
    ap.add_argument("--by-arch", action="store_true",
                    help="what each architecture costs in assembly")
    args = ap.parse_args()

    if not SRC.is_dir():
        print(f"FAIL {SRC} not found.")
        return 1

    rows: list[tuple[str, str, str, int]] = []
    for path in sorted(SRC.rglob("*")):
        if path.suffix not in (".S", ".s"):
            continue
        rel = path.relative_to(SRC).as_posix()
        verdict, label = classify(rel)
        try:
            lines = sum(1 for _ in path.open("rb"))
        except OSError:
            lines = 0
        rows.append((verdict, label, rel, lines))

    if args.by_arch:
        # What a new architecture actually costs. The irreducible column is
        # unavoidable; the eliminable column is paid again by every port, for
        # routines that already have architecture-neutral C.
        totals: dict[str, collections.Counter[str]] = collections.defaultdict(
            collections.Counter)
        for verdict, _label, rel, lines in rows:
            if verdict == "vendored":
                continue
            m = ARCH_DIR.search(rel)
            arch = ARCH_ALIAS.get(m.group(2), m.group(2)) if m else "arch-neutral"
            totals[arch][verdict] += lines
            totals[arch]["files"] += 1
        print(f"{'architecture':<14}{'files':>7}{'lines':>9}"
              f"{'irreducible':>13}{'eliminable':>12}")
        for arch in sorted(totals, key=lambda k: -sum(
                v for kk, v in totals[k].items() if kk != "files")):
            c = totals[arch]
            total = sum(v for kk, v in c.items() if kk != "files")
            print(f"{arch:<14}{c['files']:>7}{total:>9}"
                  f"{c['irreducible']:>13}{c['eliminable']:>12}")
        return 0

    if args.list:
        for verdict, label, rel, lines in rows:
            if args.list in (verdict, label):
                print(f"{lines:>6}  {rel}")
        return 0

    by_verdict: collections.Counter[str] = collections.Counter()
    lines_by_verdict: collections.Counter[str] = collections.Counter()
    by_label: collections.Counter[tuple[str, str]] = collections.Counter()
    for verdict, label, _rel, lines in rows:
        by_verdict[verdict] += 1
        lines_by_verdict[verdict] += lines
        by_label[(verdict, label)] += 1

    total = len(rows)
    total_lines = sum(lines_by_verdict.values())
    print(f"assembly files under hbsd/src: {total}  ({total_lines} lines)\n")
    for verdict in ("irreducible", "eliminable", "unclassified",
                    "test-only", "not assembly", "vendored"):
        n, ln = by_verdict[verdict], lines_by_verdict[verdict]
        if not n:
            continue
        pct = 100 * n / total
        print(f"{verdict:<14} {n:>5} files  {ln:>7} lines  ({pct:.1f}%)")
        for (v, label), count in sorted(by_label.items()):
            if v == verdict:
                print(f"    {count:>5}  {label}")
    own = total - by_verdict["vendored"]
    own_lines = total_lines - lines_by_verdict["vendored"]
    print(f"\nPBSD's own assembly: {own} files, {own_lines} lines "
          f"({100 * by_verdict['irreducible'] / own:.0f}% of it irreducible)")
    print("irreducible is the floor: no ISO C++23 construct emits these.")
    print("eliminable is the target: std::atomic, <bit>, and C fallbacks.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        sys.exit(0)
