#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Does sys/sys/atomic_generic.h emit the same instructions as the six?

tools/atomic_generic_check.py answers a narrower question - whether every
required width is lock-free - and that is a precondition, not a proof. Two
headers agreeing that atomic_add_acq_32 exists and is lock-free says
nothing about the barrier it emits, and a barrier is the whole content of
an atomic operation.

So this compiles the SAME operation twice for each target, once against
<machine/atomic.h> and once against <sys/atomic_generic.h>, at -O2, and
compares the instructions that come out. Nothing is run and no FreeBSD is
needed: the question is what the compiler emits, and the compiler is here.

What "the same" means is a decision, and it is made explicit rather than
by accident. Two sequences compare equal after:

  * dropping the assembler's comments, .cfi directives and alignment, and
    clang's //APP / //NO_APP inline-asm markers, which say where the text
    came from rather than what it is;
  * renumbering local labels in order of first appearance, so .LBB4_1 and
    .Ltmp0 are the same label in the same place;
  * joining the x86 lock prefix to the instruction it applies to, because
    the machine header writes `lock ; addl' in inline asm and the
    builtin emits `lock addl'. Same instruction, two spellings.

Everything else counts. A different barrier is a difference, a different
addressing mode is a difference, and a runtime dispatch that one side has
and the other does not is a very large difference.

The output is a verdict per (architecture, operation), and --freeze writes
them to a table so that a later change to EITHER header shows up as a
verdict that moved.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SYS = ROOT / "hbsd/src/sys"
GENERIC = "sys/atomic_generic.h"

TARGETS = {
    "amd64":   ("x86_64-unknown-freebsd15.1",  "amd64"),
    "arm64":   ("aarch64-unknown-freebsd15.1", "arm64"),
    "arm":     ("armv7-unknown-freebsd15.1-eabihf", "arm"),
    "i386":    ("i386-unknown-freebsd15.1",    "i386"),
    "powerpc": ("powerpc64-unknown-freebsd15.1", "powerpc"),
    "riscv":   ("riscv64-unknown-freebsd15.1", "riscv"),
}

# Which widths each architecture's own header implements. i386 has no
# 8- or 16-bit operations at all, and riscv starts at 32; asking for one
# that is not there is a compile error, not a difference.
WIDTHS = {
    "amd64":   (8, 16, 32, 64),
    "arm64":   (8, 16, 32, 64),
    "arm":     (8, 16, 32, 64),
    "i386":    (8, 16, 32, 64),
    "powerpc": (8, 16, 32, 64),
    "riscv":   (32, 64),
}


def wrappers(widths) -> dict[str, str]:
    """One external-linkage wrapper per operation, so the inlines emit."""
    out: dict[str, str] = {}

    def add(name: str, text: str) -> None:
        out[f"w_{name}"] = text

    for w in widths:
        t_ = f"uint{w}_t"
        for sfx in ("", "_acq", "_rel"):
            for op in ("add", "subtract", "set", "clear"):
                add(f"{op}{sfx}_{w}",
                    f"void w_{op}{sfx}_{w}(volatile {t_} *p, {t_} v)"
                    f" {{ atomic_{op}{sfx}_{w}(p, v); }}")
            add(f"cmpset{sfx}_{w}",
                f"int w_cmpset{sfx}_{w}(volatile {t_} *p, {t_} a, {t_} b)"
                f" {{ return atomic_cmpset{sfx}_{w}(p, a, b); }}")
            add(f"fcmpset{sfx}_{w}",
                f"int w_fcmpset{sfx}_{w}(volatile {t_} *p, {t_} *a, {t_} b)"
                f" {{ return atomic_fcmpset{sfx}_{w}(p, a, b); }}")
            add(f"fetchadd{sfx}_{w}",
                f"{t_} w_fetchadd{sfx}_{w}(volatile {t_} *p, {t_} v)"
                f" {{ return atomic_fetchadd{sfx}_{w}(p, v); }}")
        for sfx in ("", "_acq"):
            add(f"load{sfx}_{w}",
                f"{t_} w_load{sfx}_{w}(const volatile {t_} *p)"
                f" {{ return atomic_load{sfx}_{w}(p); }}")
        for sfx in ("", "_rel"):
            add(f"store{sfx}_{w}",
                f"void w_store{sfx}_{w}(volatile {t_} *p, {t_} v)"
                f" {{ atomic_store{sfx}_{w}(p, v); }}")
        add(f"swap_{w}", f"{t_} w_swap_{w}(volatile {t_} *p, {t_} v)"
                         f" {{ return atomic_swap_{w}(p, v); }}")
        add(f"readandclear_{w}",
            f"{t_} w_readandclear_{w}(volatile {t_} *p)"
            f" {{ return atomic_readandclear_{w}(p); }}")
    for f_ in ("acq", "rel", "acq_rel", "seq_cst"):
        add(f"fence_{f_}", f"void w_fence_{f_}(void)"
                           f" {{ atomic_thread_fence_{f_}(); }}")
    return out


# What is not an instruction. The comment character is not the same on
# every target - `#' on x86 and powerpc, `//' on arm64, `@' on arm - and
# clang wraps inline asm in a marker built from whichever it is (#APP,
# //APP, @APP). Missing one of those counts a comment as an instruction:
# before `#' was here amd64 read as 8 of 112 matching instead of 82, and
# before `@' arm's differences were inflated the same way.
#
# Directives go too, except a local label definition, which is part of the
# shape. That covers .cfi_*, .p2align, ARM's .fnstart/.cantunwind, and
# powerpc's traceback-table .long/.quad, without a list to keep current.
_COMMENT = re.compile(r"^(//|#|@|;)")
_DIRECTIVE = re.compile(r"^\.")
_LABELDEF = re.compile(r"^\.L[\w.$]*:$")


def _is_noise(s: str) -> bool:
    if _COMMENT.match(s):
        return True
    if _DIRECTIVE.match(s) and not _LABELDEF.match(s):
        return True
    return False


_LABEL = re.compile(r"\.L[A-Za-z0-9_.$]+")
_FUNC = re.compile(r"^(w_[A-Za-z0-9_]+):")


def instructions(asm: str) -> dict[str, list[str]]:
    """Each wrapper's instruction list, normalised as the docstring says."""
    out: dict[str, list[str]] = {}
    cur = None
    for raw in asm.splitlines():
        s = raw.strip()
        m = _FUNC.match(s)
        if m:
            cur = m.group(1)
            out[cur] = []
            continue
        if cur is None or not s:
            continue
        if s.startswith(".Lfunc_end"):
            cur = None
            continue
        if _is_noise(s):
            continue
        out[cur].append(re.sub(r"\s+", " ", s))

    # Rename labels per function, in order of first appearance, and join
    # the x86 lock prefix to its instruction.
    for name, body in out.items():
        seen: dict[str, str] = {}

        def renumber(m: re.Match) -> str:
            return seen.setdefault(m.group(0), f"L{len(seen) + 1}")

        body = [_LABEL.sub(renumber, line) for line in body]
        joined: list[str] = []
        for line in body:
            if joined and joined[-1].rstrip(";").strip() == "lock":
                joined[-1] = "lock " + line
            else:
                joined.append(line)
        out[name] = [x for x in joined if x.rstrip(";").strip() != "lock"]
    return out


_UNDECL = re.compile(r"call to undeclared function '(atomic_[A-Za-z0-9_]+)'")


def supported(cc: str, triple: str, incs: list[Path], header: str,
              wrappers: dict[str, str]) -> set[str]:
    """Which of these wrappers this header can actually compile.

    Not every architecture implements every operation - i386 has no 8- or
    16-bit atomics, riscv no atomic_fetchadd_acq_32 - and a hand-written
    table of which is which would be a second place for the answer to be
    wrong. So the compiler is asked: build them all, read the names it
    says are undeclared, drop those, and go again until it is quiet.
    """
    live = dict(wrappers)
    while live:
        text = (f"#include <sys/types.h>\n#include <{header}>\n"
                + "\n".join(live.values()) + "\n")
        args = [cc, "-target", triple, "-fsyntax-only", "-nostdinc",
                "-D_KERNEL", "-DSMP", "-std=gnu17", "-ferror-limit=0",
                "-x", "c", "-"]
        for i in incs:
            args[-1:-1] = [f"-I{i}"]
        r = subprocess.run(args, input=text, capture_output=True, text=True)
        if r.returncode == 0:
            break
        missing = set(_UNDECL.findall(r.stderr))
        if not missing:
            # A failure that is not a missing operation is a real one and
            # must not be silently narrowed away.
            raise RuntimeError(
                (r.stderr.strip().splitlines() or ["failed"])[0])
        before = len(live)
        live = {k: v for k, v in live.items()
                if not any(f" {m}(" in v or f"{m}(" in v for m in missing)}
        if len(live) == before:
            raise RuntimeError("cannot narrow: " + ", ".join(sorted(missing)))
    return set(live)


def compile_asm(cc: str, triple: str, incs: list[Path], header: str,
                src: str) -> tuple[str | None, str]:
    text = f"#include <sys/types.h>\n#include <{header}>\n{src}"
    args = [cc, "-target", triple, "-O2", "-S", "-nostdinc", "-D_KERNEL",
            "-DSMP", "-std=gnu17", "-x", "c", "-o", "-", "-"]
    for i in incs:
        args[-3:-3] = [f"-I{i}"]
    r = subprocess.run(args, input=text, capture_output=True, text=True)
    if r.returncode != 0:
        return None, (r.stderr.strip().splitlines() or ["failed"])[0]
    return r.stdout, ""


def digest(seq: list[str] | None) -> str:
    """A short hash of one instruction sequence.

    The verdict alone is too coarse to freeze. Weakening an acquire to a
    sequentially-consistent barrier changes the instructions and leaves
    the verdict at "different barrier", so a table of verdicts would call
    that no change at all - which is exactly what happened the first time
    this gate was tested against a deliberate edit. The digest moves when
    the code moves, on either side.
    """
    if seq is None:
        return "-"
    return hashlib.sha256("\n".join(seq).encode()).hexdigest()[:12]


def classify(arch: str, gen: list[str], mach: list[str]) -> str:
    """Name the difference, so a table of them can be read.

    Categories are recognised from what the two sequences contain, not
    from a per-operation list, because the point is to notice a new kind
    of difference rather than to confirm the known ones.
    """
    if gen == mach:
        return "same"
    if any("lse_supported" in x for x in mach):
        return "LSE dispatch (machine only)"
    bar = {
        "arm64": ("dmb", "ldar", "stlr", "ldaxr", "stlxr", "ldxr", "stxr"),
        "riscv": ("fence", ".aq", ".rl", "lr.", "sc."),
        "powerpc": ("isync", "lwsync", "sync", "eieio"),
        "arm": ("dmb", "dsb", "ldrex", "strex"),
        "amd64": ("mfence", "lfence", "sfence"),
        "i386": ("mfence", "lfence", "sfence"),
    }[arch]

    def barriers(seq):
        """The barrier-bearing MNEMONICS, without their operands.

        Comparing whole lines put `ldar w0, [x0]' against `ldar w8, [x0]'
        in this category, which is a register allocation and not a
        barrier: arm64's inline asm names its own destination and the
        builtin does not. The mnemonic is the part that says what the
        instruction orders.
        """
        out = []
        for x in seq:
            mnem = x.split()[0].rstrip(",")
            if any(b in mnem for b in bar):
                out.append(mnem)
        return out
    if barriers(gen) != barriers(mach):
        return "different barrier"
    if len(gen) != len(mach):
        return "different sequence length"
    return "different instructions"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cc", default="clang")
    ap.add_argument("--arch", action="append",
                    help="limit to these architectures (repeatable)")
    ap.add_argument("--show", metavar="OP",
                    help="print both sequences for operations matching this "
                         "substring")
    ap.add_argument("--freeze", metavar="FILE",
                    help="write the verdicts to a table, so a later change "
                         "to either header shows up as a verdict that moved")
    ap.add_argument("--baseline", metavar="FILE",
                    help="compare the verdicts against a frozen table and "
                         "fail on any that moved")
    args = ap.parse_args()

    if shutil.which(args.cc) is None:
        print(f"SKIP {args.cc} not installed; nothing measured")
        return 0

    baseline = {}
    if args.baseline:
        p = Path(args.baseline)
        if not p.is_absolute():
            p = ROOT / p
        baseline = json.loads(p.read_text())["verdicts"]

    tmp = Path(tempfile.mkdtemp(prefix="atomicgen."))
    verdicts: dict[str, dict[str, str]] = {}
    failures = 0
    print("generic vs per-architecture atomics, at -O2\n")
    print(f"{'arch':9s} {'ops':>5s} {'same':>6s} {'differ':>7s}   "
          f"kinds of difference")

    for arch, (triple, mdir) in TARGETS.items():
        if args.arch and arch not in args.arch:
            continue
        shim = tmp / arch
        shim.mkdir()
        (shim / "machine").symlink_to(SYS / mdir / "include")
        for fam in ("x86", "arm", "arm64", "powerpc", "riscv", "i386",
                    "amd64"):
            d = SYS / fam / "include"
            if d.is_dir():
                (shim / fam).symlink_to(d)
        incs = [shim, SYS]

        here = verdicts.setdefault(arch, {})
        all_w = wrappers(WIDTHS[arch])
        try:
            have_g = supported(args.cc, triple, incs, GENERIC, all_w)
            have_m = supported(args.cc, triple, incs, "machine/atomic.h",
                               all_w)
        except RuntimeError as e:
            print(f"{arch:9s}   probe: {e}")
            failures += 1
            continue
        only_g = sorted(have_g - have_m)
        only_m = sorted(have_m - have_g)
        both = sorted(have_g & have_m)
        src = "\n".join(all_w[k] for k in both) + "\n"
        g, ge = compile_asm(args.cc, triple, incs, GENERIC, src)
        m, me = compile_asm(args.cc, triple, incs, "machine/atomic.h", src)
        if g is None or m is None:
            print(f"{arch:9s}   compile: {ge or me}")
            failures += 1
            continue
        fg, fm = instructions(g), instructions(m)

        counts: dict[str, int] = {}
        for name in only_g:
            here[name] = {"v": "generic only"}
            counts["generic only"] = counts.get("generic only", 0) + 1
        for name in only_m:
            here[name] = {"v": "machine only"}
            counts["machine only"] = counts.get("machine only", 0) + 1
        for name in both:
            a, b = fg.get(name), fm.get(name)
            if a is None or b is None:
                v = "not emitted"
            else:
                v = classify(arch, a, b)
            here[name] = {"v": v, "g": digest(a), "m": digest(b)}
            counts[v] = counts.get(v, 0) + 1
            if args.show and args.show in name:
                print(f"\n  {arch} {name}")
                print(f"    generic: {'; '.join(a or ['-'])}")
                print(f"    machine: {'; '.join(b or ['-'])}")
        same = counts.pop("same", 0)
        differ = sum(counts.values())
        kinds = ", ".join(f"{k} x{v}" for k, v in sorted(counts.items()))
        print(f"{arch:9s} {same + differ:5d} {same:6d} {differ:7d}   "
              f"{kinds or '-'}")

        want = baseline.get(arch, {})
        for name, rec in here.items():
            old_ = want.get(name)
            if old_ is None:
                continue
            if old_.get("v") != rec.get("v"):
                failures += 1
                print(f"  FAIL {arch} {name}: verdict was {old_['v']!r}, "
                      f"is {rec['v']!r}")
            elif old_.get("g") != rec.get("g"):
                failures += 1
                print(f"  FAIL {arch} {name}: the GENERIC header emits "
                      f"different code ({old_.get('g')} -> {rec.get('g')})")
            elif old_.get("m") != rec.get("m"):
                failures += 1
                print(f"  FAIL {arch} {name}: <machine/atomic.h> emits "
                      f"different code ({old_.get('m')} -> {rec.get('m')})")
        for name in want:
            if name not in here:
                failures += 1
                print(f"  FAIL {arch} {name} is gone; the baseline has it "
                      f"as {want[name].get('v')!r}")

    if args.freeze:
        out = Path(args.freeze)
        if not out.is_absolute():
            out = ROOT / out
        cc_v = subprocess.run([args.cc, "--version"], capture_output=True,
                              text=True).stdout.splitlines()[0].strip()
        rev = subprocess.run(["git", "-C", str(ROOT), "rev-parse", "HEAD"],
                             capture_output=True, text=True).stdout.strip()
        out.write_text(json.dumps(
            {"_comment": "Per (architecture, operation): does "
                         "sys/sys/atomic_generic.h emit the same "
                         "instructions as <machine/atomic.h>, and if not, "
                         "what kind of difference is it. Written by "
                         "tools/atomic_codegen_check.py --freeze; checked by "
                         "--baseline. A measurement, not an edited file.",
             "compiler": cc_v, "tree": rev, "verdicts": verdicts},
            indent=1, sort_keys=True) + "\n")
        n = sum(len(v) for v in verdicts.values())
        print(f"\nfroze {n} verdict(s) to {out.relative_to(ROOT)}")

    print()
    total = sum(len(v) for v in verdicts.values())
    same = sum(1 for a in verdicts.values() for v in a.values()
               if v.get("v") == "same")
    print(f"{same} of {total} operations emit identical instructions.")
    if failures:
        print(f"{failures} verdict(s) moved against the baseline.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
