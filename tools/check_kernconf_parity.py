#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Check every architecture's HARDENEDBSD resolves to the same hardening.

sys/conf/std.hardenedbsd exists so the hardening policy is one file instead of
six. That does not by itself make the six kernels equal: each
sys/<arch>/conf/HARDENEDBSD includes GENERIC, then the shared file, then its
own overloads, and any of those can add or remove an option.

The first edit to std.hardenedbsd proved the point. INVARIANTS was moved into
std.hardenedbsd.debug, which only amd64 includes, and five architectures
silently got PAX without it - which HardenedBSD refuses to build:

    hbsd_pax_common.c:65: #error "HardenedBSD required enabled INVARIANTS"

Six kernel builds found that. This finds it in a second, by resolving the
`include` chain the way config(8) does and comparing the resulting option sets.

Resolution is deliberately literal: `include NAME` is looked up in the same
directory, `include "NAME"` in sys/conf as well, `options`/`nooptions` are
applied in order. Anything it cannot resolve is reported rather than assumed
absent, because an unresolved include is the one case where a missing option
would look like a real difference.

--all-configs asks a different question of the other twenty-one. Cross
-architecture parity is the wrong question for most of them: LATT-SEC is
amd64 only, RPI2-HARDENEDBSD is one arm board, HARDENEDBSD64 is powerpc's
64-bit kernel, and comparing a config against architectures that do not
have it says nothing. What DOES apply to every one of them is the promise
their names make. A kernel called HARDENEDBSD-something, or named for ASLR
or security, should resolve to the full hardening set wherever it lives -
and each can drift the way riscv's HARDENEDBSD-CORE did, by being edited
without the others.

Two things that mode has to get right, and got wrong first time round.

INVARIANTS is not unconditional. sys/hardenedbsd/hbsd_pax_common.c:65
refuses to build a PAX kernel without it "unless you really know what
you're doing", and the way you say so is `options PAX_INSECURE_MODE`.
amd64's HARDENEDBSD-NODEBUG and HARDENEDBSD-MINIMAL and arm64's
HARDENEDBSD-NODEBUG all carry it deliberately - a NODEBUG kernel dropping
INVARIANTS is the entire point of the config. So INVARIANTS and
INVARIANT_SUPPORT count as satisfied when PAX_INSECURE_MODE is set.

And a config whose include chain does not resolve has no option set to
judge. Five of the twenty-one include a base config that is not in the
tree - BEAGLEBONE, RPI2, RPI-B and GENERIC-NODEBUG, all removed upstream
when FreeBSD folded the individual arm boards into GENERIC. Reporting
BEAGLEBONE-ASLR as "3 of 14" is reporting on the three lines of its own
file. They are listed separately, as what they are: configs config(8)
would refuse outright.
"""

from __future__ import annotations

import argparse
import os
import re
import sys

ARCHES = ["amd64", "arm64", "arm", "i386", "powerpc", "riscv"]
INCLUDE = re.compile(r'^\s*include\s+"?([^"\s]+)"?\s*$')
OPTION = re.compile(r'^\s*(options|nooptions)\s+(\S+)')

# The options this check is about. A kernel differing on a device driver is
# expected; differing on these is the thing std.hardenedbsd exists to prevent.
HARDENING = {
    "PAX", "PAX_ASLR", "PAX_NOEXEC", "PAX_SEGVGUARD", "PAX_HARDENING",
    "PAX_SYSCTLS", "PAX_CONTROL_ACL", "PAX_CONTROL_EXTATTR",
    "PAX_CONTROL_ACL_OVERRIDE_SUPPORT", "PAX_JAIL_SUPPORT",
    "HARDEN_KLD", "HBSD_RESIST_FINGERPRINTING",
    "INVARIANTS", "INVARIANT_SUPPORT",
}


def resolve(path: str, confdir: str, sysconf: str,
            seen: set[str], unresolved: list[str]) -> list[tuple[str, str]]:
    """Return (options|nooptions, NAME) in order, following includes."""
    real = os.path.realpath(path)
    if real in seen:
        return []
    seen.add(real)
    out: list[tuple[str, str]] = []
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        unresolved.append(path)
        return []
    for line in lines:
        m = INCLUDE.match(line)
        if m:
            name = m.group(1)
            for cand in (os.path.join(confdir, name),
                         os.path.join(sysconf, name)):
                if os.path.isfile(cand):
                    out += resolve(cand, confdir, sysconf, seen, unresolved)
                    break
            else:
                unresolved.append(f"{path}: include {name}")
            continue
        m = OPTION.match(line)
        if m:
            out.append((m.group(1), m.group(2).rstrip(",")))
    return out


def effective(root: str, arch: str, conf: str,
              unresolved: list[str]) -> set[str] | None:
    confdir = os.path.join(root, "sys", arch, "conf")
    path = os.path.join(confdir, conf)
    if not os.path.isfile(path):
        return None
    sysconf = os.path.join(root, "sys", "conf")
    opts: set[str] = set()
    for kind, name in resolve(path, confdir, sysconf, set(), unresolved):
        if kind == "options":
            opts.add(name)
        else:
            opts.discard(name)
    return opts


# A config whose name makes a hardening promise. Matched on the name, not
# on what it contains, because the point is to catch one that has drifted
# out of carrying what its name says.
HARDENED_NAME = re.compile(r"HARDENEDBSD|ASLR|(?:^|-)SEC(?:$|-)")


def all_configs(root: str) -> list[tuple[str, str]]:
    """(arch, name) for every kernel config whose name promises hardening."""
    out = []
    for a in ARCHES:
        confdir = os.path.join(root, "sys", a, "conf")
        if not os.path.isdir(confdir):
            continue
        for name in sorted(os.listdir(confdir)):
            path = os.path.join(confdir, name)
            if not os.path.isfile(path) or "." in name:
                continue
            if HARDENED_NAME.search(name):
                out.append((a, name))
    return out


# INVARIANTS is required by hbsd_pax_common.c unless this is set.
INSECURE = "PAX_INSECURE_MODE"
DEBUG_OPTS = {"INVARIANTS", "INVARIANT_SUPPORT"}


def check_all(root: str, gate: bool) -> int:
    """Every hardening-named config carries the full hardening set."""
    configs = all_configs(root)
    print(f"configs whose name promises hardening: {len(configs)}\n")
    short = []
    broken = []
    for arch, name in configs:
        unresolved: list[str] = []
        opts = effective(root, arch, name, unresolved)
        if opts is None:
            continue
        if unresolved:
            broken.append((arch, name, unresolved))
            print(f"  ??   {arch:<8} {name:<24} include chain incomplete")
            continue
        required = set(HARDENING)
        if INSECURE in opts:
            required -= DEBUG_OPTS
        have = required & opts
        lacking = sorted(required - opts)
        mark = "ok  " if not lacking else "SHORT"
        note = f"  ({INSECURE})" if INSECURE in opts else ""
        print(f"  {mark} {arch:<8} {name:<24} "
              f"{len(opts):>4} options, {len(have):>2} of "
              f"{len(required)} hardening{note}")
        if lacking:
            short.append((arch, name, lacking))

    if broken:
        print(f"\n{len(broken)} include a config that is not in the tree, so "
              f"config(8) would refuse them and there is no option set to "
              f"judge:")
        for arch, name, unresolved in broken:
            miss = ", ".join(sorted({u.rsplit("include ", 1)[-1]
                                     for u in unresolved}))
            print(f"  sys/{arch}/conf/{name}  needs {miss}")

    if short:
        print(f"\n{len(short)} resolve fully and do not carry what their "
              f"name promises:")
        for arch, name, lacking in short:
            print(f"  sys/{arch}/conf/{name}")
            for o in lacking:
                print(f"    missing {o}")
    elif not broken:
        print("\nevery one carries the full hardening set.")

    return 1 if gate and (short or broken) else 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--conf", default="HARDENEDBSD")
    ap.add_argument("--all-configs", action="store_true",
                    help="instead of comparing one config across the six "
                         "architectures, check that EVERY config whose name "
                         "promises hardening resolves to the full set")
    ap.add_argument("--fail-on-difference", action="store_true")
    args = ap.parse_args()
    root = os.path.abspath(args.root)

    if args.all_configs:
        return check_all(root, args.fail_on_difference)

    unresolved: list[str] = []
    per_arch: dict[str, set[str]] = {}
    for a in ARCHES:
        got = effective(root, a, args.conf, unresolved)
        if got is None:
            print(f"  {a}: no sys/{a}/conf/{args.conf}")
            continue
        per_arch[a] = got

    if len(per_arch) < 2:
        print("need at least two architectures", file=sys.stderr)
        return 2

    print(f"{args.conf}, resolved through its include chain:\n")
    for a in sorted(per_arch):
        have = sorted(HARDENING & per_arch[a])
        print(f"  {a:<9} {len(per_arch[a]):>4} options, "
              f"{len(have):>2} of {len(HARDENING)} hardening")

    common = set.intersection(*(HARDENING & v for v in per_arch.values()))
    everywhere = {o for o in HARDENING
                  if all(o in v for v in per_arch.values())}
    missing = {}
    for o in sorted(HARDENING):
        lacking = sorted(a for a, v in per_arch.items() if o not in v)
        if lacking and len(lacking) < len(per_arch):
            missing[o] = lacking

    print(f"\nhardening options on every architecture: {len(everywhere)}")
    if missing:
        print(f"present on some and not others: {len(missing)}")
        for o, lacking in missing.items():
            print(f"  {o:<34} missing on: {', '.join(lacking)}")
    else:
        print("no hardening option differs between architectures.")

    if unresolved:
        print(f"\nunresolved includes ({len(unresolved)}) - these could hide "
              f"a difference:")
        for u in unresolved[:10]:
            print(f"  {u}")

    if (missing or unresolved) and args.fail_on_difference:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
