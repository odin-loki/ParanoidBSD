#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A FuSeBMC-architecture engine: bounded model checking seeds a fuzzer.

WHAT FuSeBMC IS, AND WHAT THIS IS

FuSeBMC (Alshmrany et al., SV-COMP) is a hybrid: it runs a bounded model
checker to produce counterexample inputs, hands those to a fuzzer as
"smart seeds", and feeds coverage the fuzzer discovers back as new
goals. The two halves cover each other's blind spots -- BMC reasons
about all paths to a shallow depth, fuzzing reaches deep paths it can
stumble into but proves nothing.

The published FuSeBMC is ESBMC + Map2Check + AFL behind a Python driver,
distributed from GitHub. This container's network policy scopes GitHub
to one owner, and ESBMC is in no distribution's package set, so neither
is reachable here. What IS here is CBMC 5.95.1 with goto-cc, and AFL++
4.09c from the archive -- which is the same architecture with a
different BMC engine.

So this is not FuSeBMC. It is FuSeBMC's method, built on the model
checker this tree already uses, and it is named for the method.

WHAT IT PROVES

Nothing. Say that first and keep saying it.

CBMC with --unwind K and --unwinding-assertions can PROVE a bounded
property (cbmc_driver.py's vocabulary, and this reuses it). Fuzzing
cannot: a fuzzer that finds nothing has told you that it found nothing.
The value is asymmetric and worth having anyway -- a crash is a
certainty, and the crashes a fuzzer finds are ones the analyser and the
model checker both missed, because it runs the code.

The status vocabulary is never merged:

  CRASH       the compiled function died on a concrete input, which is
              recorded. ASan/UBSan are on, so this includes reads and
              writes CBMC models and clang's analyser guesses at.
  CLEAN       the fuzzer ran its full budget and found nothing. NOT a
              proof, and never reported as one.
  NOSEED      CBMC could not be run on this function, so the fuzzer got
              no smart seed and was not started. The BMC half is what
              makes this different from blind fuzzing; without it this
              is not the method.
  ERROR       the function could not be compiled into a runnable
              harness. This is the common case and the honest number.
  NOFUNC      not in this translation unit.
  TIMEOUT     the budget expired mid-run with the corpus not drained.
  NORETURN    the function ends the process rather than returning. AFL's
              child does not come back to the fork server whether it
              exited or died, so a fuzzer cannot tell those apart, and
              this one is not run rather than reported as a crash it
              cannot distinguish. _Exit() and quick_exit() were two of
              the first four CRASHes this ever produced at scale.
  SANFAIL     the sanitizer aborted on its OWN internal invariant --
              "AddressSanitizer: CHECK failed: asan_allocator.cpp:601"
              -- which says nothing about the code under it. Kept
              separate rather than dropped, because it is a real limit
              on what this engine can see. memalign(1 << 63, 0) was the
              first.
  RUNNABLE    --dry-run only. The harness WOULD build; nothing was run
              and nothing is claimed. It is the reach number, and it is
              a ceiling on the truth -- a function that classifies
              RUNNABLE can still fail to compile or link.

WHY ERROR IS THE COMMON CASE, AND WHY THAT IS THE MEASUREMENT

cbmc_driver.py explains it for CBMC: hbsd/src is FreeBSD source, its
headers collide with glibc's on a Linux host, and most translation units
cannot be compiled standalone here at all. This engine needs strictly
more than CBMC does -- CBMC needs to PARSE the unit, this needs to
COMPILE, LINK and RUN it -- so its reach is a subset of CBMC's reach,
which is itself 749 of 4737 units.

That ratio is the result, not an embarrassment to be hidden. A tool that
reported a number without saying which functions it never reached would
be worse than useless here.

THE HARNESS, AND ITS LIMIT

A fuzzer needs an entry point that turns bytes into arguments. This
generates one for functions whose parameters are all scalars -- the same
SCALAR class cbmc_driver.py allows by default -- by reading sizeof(T)
bytes per parameter from stdin:

    int main(void) {
        uint8_t buf[N];
        if (fread(buf, 1, sizeof buf, stdin) != sizeof buf) return 0;
        (void)F(*(int32_t *)(buf + 0), *(uint64_t *)(buf + 4));
        return 0;
    }

A pointer parameter is NOT synthesised. cbmc_driver.py runs those under
an explicit stated precondition (--min-null-tree-depth) and records the
assumption; there is no honest equivalent for a fuzzer, which would
either pass NULL (reporting the absence of a caller's contract as a
crash) or a buffer of an invented size (reporting the invention). Those
functions come back ERROR with the reason, which is true.

DRIVING IT OVER THE TREE

--src/--function names one pair. Everything else works from the port
ledger (docs/port_plan.json), one task per (translation unit, function
the ledger says that unit defines):

  --plan P      the ledger. Default docs/port_plan.json.
  --scope DIR   a path prefix under hbsd/src, repeatable.
  --limit N     stop after N pairs.
  --jobs N      fuzz N functions at once. Each burns --budget seconds of
                wall clock, so this is the only knob that makes a whole
                scope finish in a day.
  --resume      append to --out, skipping pairs already recorded in it.
  --dry-run     classify only. parse_params() is the whole gate and it
                costs a regex, so the reach of this engine can be
                measured over the whole tree without spending a fuzzing
                budget per function to find it out.

load_tasks() deliberately does NOT intersect the ledger with a goto
model the way cbmc_driver.py does. A function this engine cannot
synthesise parameters for is an ANSWER -- ERROR, with the reason --
not a task to drop before counting. Drop them silently and the reach
number stops meaning anything, which is the failure this whole tool is
written against. So the run ends with a histogram of the ERROR reasons,
bucketed on the reason with the quoted parameter stripped out, and that
histogram is the actual result of a whole-tree run.
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
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import noreturn_check                                    # noqa: E402
from includes import (arch_of, include_flags, lang_flags,   # noqa: E402
                      libcxx_shim)

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"


def terminates_process(path: Path, fn: str) -> bool:
    """Does fn end the process rather than return?

    A fuzzer cannot tell "the function called exit()" from "the function
    crashed": AFL's child never comes back to the fork server either
    way.  _Exit() and quick_exit() were both reported CRASH by the first
    run of this at scale -- two of its four -- and a tool whose headline
    claim is "a crash is a certainty" cannot afford a fifty per cent
    false rate on the one status that matters.

    The predicate is noreturn_check.py's, reused rather than rewritten:
    the SEED set of libc names that do not return, propagated through
    functions whose last top-level statement calls one of them, plus
    anything a header declares __dead2.
    """
    if fn in noreturn_check.SEED:
        return True
    try:
        raw = path.read_text(errors="surrogateescape")
    except OSError:
        return False
    text = noreturn_check.strip_noise(raw)
    lines = text.split("\n")
    ends_with = {}
    for name, _start, body, end in noreturn_check.definitions(lines):
        callee = noreturn_check.last_call(lines, body, end)
        if callee:
            ends_with[name] = callee
    if fn not in ends_with:
        # It may still be declared __dead2 without ending in a bare call
        # -- a `for (;;)' that never breaks, say.
        return bool(noreturn_check.declared_noreturn(
            text + "\n" + noreturn_check.headers_for(path), fn))
    noreturn = set(noreturn_check.SEED) | noreturn_check.header_noreturn(path)
    changed = True
    while changed:
        changed = False
        for name, callee in ends_with.items():
            if name not in noreturn and callee in noreturn:
                noreturn.add(name)
                changed = True
    return fn in noreturn

# Scalars a harness can synthesise by reading bytes. Anything else is a
# reason to stop, not a reason to guess.
SCALAR = {
    "char": 1, "signed char": 1, "unsigned char": 1,
    "short": 2, "unsigned short": 2, "short int": 2,
    "int": 4, "unsigned": 4, "unsigned int": 4, "signed int": 4,
    "long": 8, "unsigned long": 8, "long int": 8,
    "long long": 8, "unsigned long long": 8,
    "int8_t": 1, "uint8_t": 1, "int16_t": 2, "uint16_t": 2,
    "int32_t": 4, "uint32_t": 4, "int64_t": 8, "uint64_t": 8,
    "size_t": 8, "ssize_t": 8, "off_t": 8, "intmax_t": 8, "uintmax_t": 8,
    # A boolean is a scalar and its bytes are not free -- see BOOLEAN.
    "_Bool": 1, "bool": 1,
    # Floating point.  A random bit pattern is a NaN or a denormal about
    # as often as not, which is a fine thing to hand a function that
    # takes a double and a thing most callers never do.
    "float": 4, "double": 8,
    # The BSD spellings and the POSIX typedefs.  Every one of these was
    # an ERROR line reading "parameter type 'u_int' is not a known
    # scalar" -- a true statement about the table, not about the type.
    "u_char": 1, "u_short": 2, "u_int": 4, "u_long": 8,
    "u_int8_t": 1, "u_int16_t": 2, "u_int32_t": 4, "u_int64_t": 8,
    "uint": 4, "ushort": 2, "uchar": 1,
    "mode_t": 2, "pid_t": 4, "uid_t": 4, "gid_t": 4, "id_t": 4,
    "dev_t": 8, "ino_t": 8, "nlink_t": 8, "time_t": 8,
    "blkcnt_t": 8, "blksize_t": 8, "fsblkcnt_t": 8, "fsfilcnt_t": 8,
    "rlim_t": 8, "useconds_t": 4, "suseconds_t": 8, "clock_t": 4,
    "socklen_t": 4, "in_addr_t": 4, "in_port_t": 2, "sa_family_t": 1,
    "key_t": 4, "wchar_t": 4, "wint_t": 4, "ptrdiff_t": 8,
    "daddr_t": 8, "lwpid_t": 4, "fixpt_t": 4, "segsz_t": 8,
    # The kernel's address-sized integers.  sys/sys/_types.h defines
    # __vm_offset_t as __uint64_t (or __uint32_t on the 32-bit
    # architectures); they are integers, not the opaque pointers the
    # names beside them in the ERROR histogram are.  The width here is
    # the LP64 one, which is what this host compiles.
    "vm_offset_t": 8, "vm_paddr_t": 8, "vm_size_t": 8, "vm_ooffset_t": 8,
    "vm_pindex_t": 8, "vm_prot_t": 1, "vm_memattr_t": 4,
    "bus_addr_t": 8, "bus_size_t": 8, "bus_space_handle_t": 8,
    "register_t": 8, "u_register_t": 8, "db_expr_t": 8, "db_addr_t": 8,
    "cap_rights_t": 8, "sbintime_t": 8, "bintime_t": 8,
    # An FDT phandle is a uint32.  long double is 16 bytes on amd64 and
    # the harness memcpy()s whatever width the table says.
    "phandle_t": 4, "pcell_t": 4, "long double": 16,
    "uintptr_t": 8, "intptr_t": 8,
    # The Linux-compat spellings the drm and ofed trees use.
    "u8": 1, "u16": 2, "u32": 4, "u64": 8,
    "s8": 1, "s16": 2, "s32": 4, "s64": 8,
    "__u8": 1, "__u16": 2, "__u32": 4, "__u64": 8,
}

# Typedefs that name a POINTER.  Without these the histogram called
# 9,559 device_t parameters "not a known scalar", which is true of the
# table and false about the type -- and this tool's whole claim is that
# the ERROR breakdown is the measurement.  A pointer is a pointer, and
# the reason it cannot be synthesised is the same one the `*' spelling
# gets.
POINTER_TYPEDEF = {
    "device_t", "if_t", "node_p", "hook_p", "kobj_t", "if_ctx_t",
    "module_t", "linker_file_t", "devclass_t", "driver_t", "bus_dma_tag_t",
    "bus_dmamap_t", "CpaInstanceHandle", "ifnet_t", "vnode_t", "cred_t",
    "kthread_t", "proc_t", "file_t", "zfs_file_t", "taskqueue_t",
    "eventhandler_tag", "sbuf_t", "mbuf_t", "callout_t", "malloc_type_t",
    "vm_object_t", "vm_page_t", "vm_map_t", "pmap_t", "platform_t",
    "POCE_SOFTC", "SCI_CONTROLLER_HANDLE_T", "SCI_OBJECT_HANDLE_T",
    "objset_t", "dsl_pool_t", "spa_t", "dmu_tx_t", "znode_t",
    "ib_device_t", "ocs_t", "ocs_hw_t", "efct_t", "adapter_t",
    "caddr_t", "ACPI_HANDLE", "ACPI_TABLE_HEADER",
}

# A parameter that is really a MACRO standing for a whole parameter
# list.  SYSCTL_HANDLER_ARGS alone is 1,140 of them.  Calling that
# "not a known scalar" says nothing; naming it says what would have to
# change for the engine to reach these at all.
PARAM_MACRO = {
    "SYSCTL_HANDLER_ARGS", "PFS_FILL_ARGS", "PFS_ATTR_ARGS",
    "PFS_VIS_ARGS", "PFS_IOCTL_ARGS", "PFS_CLOSE_ARGS", "PFS_DESTROY_ARGS",
}

# _Bool is the one scalar whose bytes are not free.  memcpy()ing an
# arbitrary byte into a _Bool can make a trap representation, and reading
# it back is undefined -- so the harness would be the bug, and every
# CRASH it reported would be its own.  Normalise instead.
BOOLEAN = {"_Bool", "bool"}
CTYPE = {1: "uint8_t", 2: "uint16_t", 4: "uint32_t", 8: "uint64_t"}

# The return type sits on its own line in KNF -- `int\nffs(int mask)\n{'
# -- so the character class before the name has to admit a newline. It did
# not, and ffs(), the first function cbmc_driver.py was ever run on, came
# back as if it had a pointer parameter.
PROTO_RE_TMPL = (
    r'(?:^|\n)(?P<ret>[A-Za-z_][\w \t*\n]*?)\b{fn}\s*\((?P<args>[^;{{]*)\)'
    r'\s*\n?\s*\{{'
)


def parse_params(text: str, fn: str):
    """Parameter (type, name) list for fn.

    Returns None when no definition was found at all, and the string
    reason when one was found but cannot be harnessed -- the caller
    reports those differently, because "this file does not define it"
    and "this takes a pointer" are not the same answer.
    """
    m = re.search(PROTO_RE_TMPL.format(fn=re.escape(fn)), text)
    if not m:
        return None
    args = m.group("args").strip()
    if args in ("", "void"):
        return []
    out = []
    depth = 0
    cur = []
    for ch in args:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur))
            cur = []
            continue
        cur.append(ch)
    out.append("".join(cur))
    params = []
    for a in out:
        a = a.strip()
        if not a:
            return "could not split the parameter list"

        if "*" in a or "[" in a or "..." in a:
            return ("a pointer, array or varargs parameter (%s) cannot be "
                    "synthesised without inventing its object" % a)
        a = re.sub(r'\b(const|volatile|register|_Atomic)\b', ' ', a).strip()
        # `int signo __unused' is an int named signo, not a type called
        # "int signo".  The attribute has to come off BEFORE the last
        # token is taken for the name: 148 parameters came back as
        # "parameter type 'int signo' is not a known scalar", which is a
        # true statement about the table and a false one about the
        # parameter -- the same shape of mistake as the newline in the
        # return type that made ffs() look like it took a pointer.
        a = re.sub(r'__attribute__\s*\(\(.*?\)\)', ' ', a)
        a = re.sub(r'\b__unused\b|\b__packed\b|\b__restrict\b'
                   r'|\b__aligned\s*\([^)]*\)', ' ', a).strip()
        toks = a.split()
        if len(toks) < 2:
            # `int' with no name, or a typedef this does not know
            ty, nm = " ".join(toks), "a%d" % len(params)
        else:
            ty, nm = " ".join(toks[:-1]), toks[-1]
        ty = re.sub(r'\s+', ' ', ty).strip()
        if ty in PARAM_MACRO:
            return ("%s is a macro standing for a whole parameter list, "
                    "not a type this can synthesise" % ty)
        if ty in POINTER_TYPEDEF:
            return ("a pointer parameter (%s) cannot be synthesised "
                    "without inventing its object" % a)
        if ty not in SCALAR:
            return "parameter type %r is not a known scalar" % ty
        params.append((ty, nm))
    return params


def layout(params):
    """[(offset, width, type, name)] and the total byte count.

    The harness reads a parameter from this offset and the seed packer
    writes it to the same one. They were separate before, and a
    counterexample of INT_MIN reached the target as a zero byte -- which
    is the difference between FuSeBMC's "smart seed" and a random one.
    """
    off, out = 0, []
    for i, (ty, nm) in enumerate(params):
        w = SCALAR[ty]
        off += (-off) % w                       # natural alignment
        out.append((off, w, ty, nm))
        off += w
    return out, max(off, 1)


def return_type(text: str, fn: str) -> str:
    m = re.search(PROTO_RE_TMPL.format(fn=re.escape(fn)), text)
    if not m:
        return "int"
    r = re.sub(r'\s+', ' ', m.group("ret")).strip()
    r = re.sub(r'\b(static|inline|__inline|extern|__pure|__const)\b', ' ', r)
    return re.sub(r'\s+', ' ', r).strip() or "int"


def build_harness(src: Path, fn: str, params, ret: str = "int") -> str:
    """C source for a stdin-driven entry point calling fn."""
    slots, total = layout(params)
    calls, decls = [], []
    for i, (off, w, ty, _nm) in enumerate(slots):
        if ty in BOOLEAN:
            decls.append("\t%s v%d = (buf[%d] & 1) != 0;" % (ty, i, off))
        else:
            decls.append("\t%s v%d;\n\tmemcpy(&v%d, buf + %d, %d);"
                         % (ty, i, i, off, w))
        calls.append("v%d" % i)
    return (
        '#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n\n'
        '/* Generated by tools/verify/fusebmc.py -- do not edit. */\n'
        'extern %s %s();\n'
        '%s\n'
        'int main(void)\n{\n'
        '\tuint8_t buf[%d];\n\n'
        '\tif (fread(buf, 1, sizeof buf, stdin) != sizeof buf)\n'
        '\t\treturn (0);\n%s\n'
        '\t%s%s(%s);\n'
        '\treturn (0);\n}\n'
        % (ret, fn,
           "" if ret == "void" else
           ("/*\n"
            " * The result goes into a volatile sink. Discarded, it lets\n"
            " * the optimiser see the definition is pure -- both files are\n"
            " * in one translation unit here -- and delete the call\n"
            " * outright, which is a full fuzzing budget spent on nothing.\n"
            " */\n"
            "static volatile %s sink;" % ret),
           total, "\n".join(decls),
           "" if ret == "void" else "sink = ", fn, ", ".join(calls))
    )


def cpp_flags(flags: list[str], accept=("-I", "-D", "-U")) -> list[str]:
    """The preprocessor half of a flag list, with its pairs kept whole.

    `-include foo.h' is TWO argv elements, so a filter written as a list
    comprehension over the first character keeps the `-include' and
    drops the header -- which is worse than dropping both, because the
    next flag silently becomes its argument.

    `accept' narrows it further for cbmc, which is not a compiler
    driver: it takes -I and -D and rejects everything else outright.
    Passing it -U__linux__ makes it print its usage and exit, and the
    engine read that as "no seed" -- six functions came back NOSEED with
    a page of cbmc's own help text as the reason, which is the tool
    reporting its own misuse as a property of the code.
    """
    keep, i = [], 0
    pair = ("-include", "-I", "-D", "-U", "-isystem", "-imacros")
    while i < len(flags):
        f = flags[i]
        if f in pair:                       # separated form: -I dir
            if f in accept:
                keep += flags[i:i + 2]
            i += 2
            continue
        if f.startswith(accept):
            keep.append(f)
        i += 1
    return keep


SAN_INTERNAL = re.compile(
    r"(AddressSanitizer|UndefinedBehaviorSanitizer|LeakSanitizer|"
    r"ThreadSanitizer|MemorySanitizer|Sanitizer): CHECK failed:")


def evidence(text: str) -> str:
    """The part of a sanitizer report that says what happened.

    _tail() is right for a compiler, which puts its verdict last, and
    wrong for a sanitizer, which puts it FIRST and a stack trace after
    it. A 600-character tail of an ASan report is the frame numbers.
    """
    m = SAN_INTERNAL.search(text or "")
    return (text[m.start():m.start() + 400] if m else _tail(text, 600))


def sanitizer_broke(text: str) -> bool:
    """Did the SANITIZER abort on its own invariant rather than report?

        AddressSanitizer: CHECK failed: asan_allocator.cpp:601
        "((user_end)) <= ((alloc_end))" (0x8000000000000001, ...)

    That is not a finding about the program. memalign(1 << 63, 0) came
    back CRASH on it, and the C library's answer to an alignment it does
    not support is NULL with EINVAL -- so the report was the
    instrument's arithmetic overflowing, not the code's. A crash that is
    the instrument is worse than no crash at all, because it is spent as
    if it were the certainty this engine's whole value rests on.
    """
    return bool(SAN_INTERNAL.search(text or ""))


def flags_for(src: Path) -> list[str]:
    """The include path this translation unit is actually compiled with.

    The first version of this engine passed NONE of it -- neither to
    CBMC nor to the harness compiler -- and the cost was not a rounding
    error. Over lib/libc and lib/msun, 854 of 4,390 pairs came back
    NOSEED and the top four reasons were `namespace.h: No such file or
    directory' (201), `sys/_types.h' (137), `fpmath.h' (112) and a
    CONVERSION ERROR that is the same thing one layer down. All four
    headers are IN THIS TREE; nothing was wrong with the code, the
    instrument was reading it through a Linux host's include path.

    classify.py has computed this per file since the beginning, for
    goto-cc. It is the same answer for cbmc and for afl-clang-fast, so
    it is asked for the same way rather than approximated a third time.
    """
    try:
        rel = src.resolve().relative_to(SRC).as_posix()
    except ValueError:
        rel = str(src)
    # as_c=True: a landed .cpp port is a pure rename at this stage, and
    # the harness is C. classify.py makes the same choice for the same
    # reason.
    flags = (lang_flags(src, rel, as_c=True)
             + include_flags(src, arch_of(rel), cc="clang"))
    if src.suffix == ".cpp":
        # ...and having said the file is C, do not then put libc++ in
        # front of the C headers. include_flags() leads every .cpp with
        # the libc++ shim, which is right for a C++ compile and fatal
        # for a C one: <__config> #errors by name --
        #
        #   libc++ only supports C++03 with Clang-based compilers.
        #   Please enable C++11
        #
        # -- through cbmc's GCC preprocessing, and imaxabs.cpp went from
        # a counterexample (the real imaxabs(INTMAX_MIN)) to NOSEED the
        # moment the include path was wired up. A widening that loses a
        # finding is not a widening.
        drop = set(libcxx_shim())
        flags = [f for f in flags if f not in drop]
    return flags


def cbmc_seed(src: Path, fn: str, params, nbytes: int, timeout: int):
    """A seed from CBMC: its counterexample values if it has one, else None.

    This is the half that makes it FuSeBMC's method rather than fuzzing.
    A seed that lands on a path CBMC already reached is worth more than a
    random one, and when CBMC PROVED the function the fuzzer is being
    asked to look somewhere BMC has already closed -- which is exactly
    when its budget should go elsewhere.
    """
    base = ["cbmc", str(src), "--function", fn, "--unwind", "4",
            "--bounds-check", "--pointer-check", "--div-by-zero-check",
            "--signed-overflow-check", "--trace"]
    # -I and -D and nothing else: see cpp_flags(). cbmc does not need
    # -nostdinc either -- it models the C library itself rather than
    # reading a host's headers for it.
    #
    # TWO attempts, the flagged one first. The build's include path is
    # a large net win (854 NOSEED to 246 over lib/libc and lib/msun,
    # because namespace.h and fpmath.h are in this tree) but it is not
    # a pure one: it also hands cbmc's C front end headers cbmc cannot
    # parse, and imaxabs.cpp went from a real counterexample --
    # imaxabs(INTMAX_MIN) -- to `parse error before __char16_t' the
    # moment sys/_types.h became reachable. A widening that loses a
    # finding is not a widening, so where the flagged run cannot even
    # parse, the bare one it used to do is tried instead and whichever
    # answers is kept.
    attempts = [cpp_flags(flags_for(src), accept=("-I", "-D")), []]
    out = ""
    for i, extra in enumerate(attempts):
        if i and extra == attempts[0]:      # identical: nothing to retry
            break
        try:
            p = subprocess.run(base + extra, capture_output=True, text=True,
                               timeout=timeout)
        except (subprocess.TimeoutExpired, OSError):
            return None, "cbmc did not run"
        out = (p.stdout or "") + (p.stderr or "")
        if "PARSING ERROR" not in out and "CONVERSION ERROR" not in out:
            break
    if "VERIFICATION FAILED" in out:
        # CBMC prints `  j=-2147483648 (10000000 ...)' for each input.
        # Pack each named value at the width and offset the harness reads
        # it from -- truncating to a byte, as this first did, discards
        # exactly the extreme values a counterexample is made of.
        seen = {m.group(1): int(m.group(2)) for m in re.finditer(
            r'^\s*([A-Za-z_]\w*)=(-?\d+)\s*\(', out, re.M)}
        buf = bytearray(nbytes)
        slots, _ = layout(params)
        got = False
        for off, w, _ty, nm in slots:
            if nm in seen:
                buf[off:off + w] = (seen[nm] & ((1 << (8 * w)) - 1)) \
                    .to_bytes(w, "little")
                got = True
        if got:
            return bytes(buf), "counterexample"
        return bytes(buf), "counterexample-unmapped"
    if "VERIFICATION SUCCESSFUL" in out:
        return bytes(nbytes), "proved-within-bound"
    return None, _tail(out)


def _tail(s: str, n: int = 300) -> str:
    return s.strip()[-n:]


def fuzz_one(src: Path, fn: str, budget: int, workdir: Path,
             cbmc_timeout: int) -> dict:
    rec = {"file": str(src.relative_to(SRC)), "function": fn}
    try:
        text = src.read_text(errors="replace")
    except OSError as e:
        return {**rec, "status": "ERROR", "detail": str(e)}

    params = parse_params(text, fn)
    if params is None:
        return {**rec, "status": "NOFUNC",
                "detail": "no definition of %s in this translation unit" % fn}
    if isinstance(params, str):
        return {**rec, "status": "ERROR", "detail": params}
    if terminates_process(src, fn):
        return {**rec, "status": "NORETURN",
                "detail": "this function ends the process; a fuzzer cannot "
                          "tell that from a crash, so it is not run"}

    harness = build_harness(src, fn, params, return_type(text, fn))
    nbytes = int(re.search(r'uint8_t buf\[(\d+)\]', harness).group(1))

    work = workdir / ("%s_%s" % (src.stem, fn))
    shutil.rmtree(work, ignore_errors=True)
    (work / "in").mkdir(parents=True)
    (work / "harness.c").write_text(harness)

    t_start = time.time()
    seed, why = cbmc_seed(src, fn, params, nbytes, cbmc_timeout)
    if seed is None:
        return {**rec, "status": "NOSEED", "detail": why}
    (work / "in" / "seed").write_bytes(seed)

    cc = shutil.which("afl-clang-fast") or shutil.which("afl-cc")
    if cc is None:
        return {**rec, "status": "ERROR", "detail": "no afl compiler"}
    binp = work / "target"
    env = {**os.environ, "AFL_QUIET": "1", "AFL_LLVM_INSTRUMENT": "CLASSIC"}
    # -fno-builtin is not optional and the reason is worth stating.
    #
    # abs() is a compiler builtin. Without it, clang recognised the call
    # in the harness, lowered it to llvm.abs with is_int_min_poison=false,
    # and NEVER CALLED the definition in abs.c -- so the engine ran a
    # full fuzzing budget against the compiler's implementation and
    # reported CLEAN on a function it had not executed. The same is true
    # of memcpy, strlen, memset and every other libc name clang knows.
    # A harness without -fno-builtin is testing the compiler.
    #
    # HOST headers here, deliberately, and NOT the tree's -- which is
    # the opposite of what cbmc_seed() is given three lines up, so it is
    # worth saying why.
    #
    # The two halves of this hybrid want different things. CBMC READS
    # the code: giving it flags_for(src) is strictly better, because
    # namespace.h and fpmath.h are in this tree and a header it cannot
    # find is a function it cannot model. The fuzzer RUNS the code, on
    # Linux, linked against glibc. Compiling the unit with the build's
    # own flags was tried and it does not work: those flags carry
    # --target=x86_64-unknown-freebsd15.0, the object comes out with a
    # FreeBSD ABI, and linking it into a Linux binary produced three
    # CRASHes that were all the harness -- valloc(0) and weekday(729652)
    # both SEGV'd before executing a line of their own code. A crash
    # that is the instrument is worse than no crash at all.
    #
    # So the reach of the fuzzing half stays bounded by what compiles
    # and links against the host's C library, and that bound is reported
    # as ERROR with the linker's own words rather than papered over.
    cp = subprocess.run(
        [cc, "-g", "-O1", "-fno-builtin", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(work / "harness.c"), str(src),
         "-o", str(binp)],
        capture_output=True, text=True, env=env)
    if cp.returncode != 0:
        return {**rec, "status": "ERROR",
                "detail": _tail((cp.stdout or "") + (cp.stderr or ""))}

    # PHASE: replay the BMC seed before fuzzing.
    #
    # This is the point of the hybrid and it is easy to miss. When CBMC
    # hands back a counterexample, that counterexample IS an input that
    # violates a property -- so running it is the cheapest possible test,
    # and if it dies there is nothing left to fuzz for. AFL++ also
    # refuses to start at all when every seed crashes ("We need at least
    # one valid input seed that does not crash!"), which the first
    # version of this read as CLEAN. A fuzzer that would not start and a
    # fuzzer that found nothing are not the same answer.
    #
    # allocator_may_return_null=1, because without it ASan ABORTS on a
    # request its allocator considers absurd -- and "absurd" is exactly
    # what a fuzzer feeds a function whose parameter is a size. valloc()
    # and memalign() were both reported CRASH on the first real run over
    # lib/libc, on 0x8700000000 and on an alignment of 2^63; the C
    # library returns NULL with ENOMEM there, and reporting that as a
    # crash is reporting the sanitizer's policy as the code's defect.
    # With the option set the call returns NULL, and a function that
    # then USES that NULL still crashes, which is the finding worth
    # having.
    SAN = {"UBSAN_OPTIONS": "halt_on_error=1:abort_on_error=1:print_stacktrace=1",
           "ASAN_OPTIONS": "abort_on_error=1:symbolize=0:detect_leaks=0"
                           ":allocator_may_return_null=1"}
    rp = subprocess.run([str(binp)], input=seed, capture_output=True,
                        timeout=30, env={**os.environ, **SAN})
    # A SIGNAL, not a status.  The harness always returns 0, so a
    # non-zero EXIT can only come from the function choosing it -- which
    # is a function doing its job, not a crash.  Both sanitizers are
    # configured abort_on_error=1, so a real report arrives as SIGABRT.
    if rp.returncode < 0:
        said = (rp.stderr or b"").decode("utf-8", "replace")
        return {**rec,
                "status": "SANFAIL" if sanitizer_broke(said) else "CRASH",
                "seed": why, "found_by": "bmc-seed",
                "inputs": [seed.hex()], "detail": evidence(said),
                "elapsed": time.time() - t_start}

    # A second, all-zero seed so the corpus survives one bad input.
    (work / "in" / "zero").write_bytes(bytes(nbytes))

    out = work / "out"
    # UBSan exits 1 by default, and AFL detects crashes by SIGNAL -- so
    # without abort_on_error a UBSan report is invisible to the fuzzer.
    env2 = {**env, **SAN, "AFL_SKIP_CPUFREQ": "1", "AFL_NO_AFFINITY": "1",
            "AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES": "1",
            "AFL_BENCH_UNTIL_CRASH": "1"}
    t0 = time.time()
    try:
        ap_ = subprocess.run(
            ["afl-fuzz", "-i", str(work / "in"), "-o", str(out),
             "-V", str(budget), "-m", "none", "--", str(binp)],
            capture_output=True, text=True, timeout=budget + 60, env=env2)
        aflout = (ap_.stdout or "") + (ap_.stderr or "")
        (work / "afl.log").write_text(aflout)
        # A fuzzer that refused to start is not a fuzzer that found
        # nothing, and the two must never come back as the same word.
        if "PROGRAM ABORT" in aflout or "All test cases processed." \
                not in aflout:
            return {**rec, "status": "ERROR",
                    "detail": "afl-fuzz did not start: " + _tail(aflout)}
    except subprocess.TimeoutExpired:
        return {**rec, "status": "TIMEOUT", "elapsed": time.time() - t0}
    except OSError as e:
        return {**rec, "status": "ERROR", "detail": str(e)}

    crashes = sorted((out / "default" / "crashes").glob("id:*")) \
        if (out / "default" / "crashes").is_dir() else []
    if crashes:
        # REPLAY the first one and keep what the sanitizer said.
        #
        # AFL saves the input and nothing else, so a fuzzer-found CRASH
        # was recorded as a hex string with an empty `detail' -- which
        # names an input without naming a defect, and every one of them
        # then had to be re-run by hand to find out what it was. The
        # seed-replay path a hundred lines up already keeps the
        # sanitizer's report; this is the same thing for the other half,
        # and it costs one execution.
        detail, said, first = "", "", crashes[0].read_bytes()
        try:
            back = subprocess.run([str(binp)], input=first,
                                  capture_output=True, timeout=30,
                                  env={**os.environ, **SAN})
            # `said' is the WHOLE report and `detail' its tail. The
            # sanitizer prints its own CHECK failure on the FIRST line
            # and the stack after it, so a tail of 600 characters is
            # exactly the part that does not contain the answer --
            # memalign came back CRASH again with the line that would
            # have classified it three screens above the cut.
            said = (back.stderr or b"").decode("utf-8", "replace")
            detail = _tail(said, 600)
            if sanitizer_broke(said):
                detail = evidence(said)
            elif back.returncode >= 0:
                # It did not die this time. Say so rather than dressing
                # a stale artefact up as a reproduction: AFL's own
                # environment differs from a bare run, and an input that
                # only crashes under it is a claim about the fuzzer.
                detail = ("did not reproduce outside afl-fuzz (exit %d); "
                          "%s" % (back.returncode, detail))
        except (subprocess.TimeoutExpired, OSError) as e:
            detail = "replay failed: %r" % (e,)
        return {**rec,
                "status": "SANFAIL" if sanitizer_broke(said) else "CRASH",
                "seed": why, "found_by": "fuzzer",
                "inputs": [c.read_bytes().hex() for c in crashes[:4]],
                "detail": detail, "elapsed": time.time() - t0}
    return {**rec, "status": "CLEAN", "seed": why,
            "elapsed": time.time() - t0,
            "note": "found nothing in this budget; not a proof"}


def load_tasks(plan: Path, scopes: list[str], limit: int | None = None):
    """One task per (translation unit, function the ledger says it defines).

    cbmc_driver.py intersects the ledger with classify.py's goto model,
    because CBMC needs a model to check and a function absent from one is
    not a task.  This does NOT, and the difference is deliberate: this
    engine derives a function's parameter class from the SOURCE, and a
    function whose parameters it cannot synthesise is an ANSWER --
    ERROR with the reason -- not a task to drop before counting.  Drop
    them silently and the reach number stops meaning anything, which is
    the failure this whole tool is written against.
    """
    d = json.loads(plan.read_text())
    tasks = []
    for rec in d["records"]:
        path = rec.get("path") or ""
        if scopes and not any(path.startswith(s) for s in scopes):
            continue
        for fn in rec.get("functions") or ():
            tasks.append({"file": path, "function": fn})
            if limit and len(tasks) >= limit:
                return tasks
    return tasks


def classify_one(t: dict) -> dict:
    """What fuzz_one() would decide before it spends anything.

    parse_params() is the whole gate -- it reads the source and answers
    NOFUNC, ERROR-with-a-reason, or a parameter list -- and it costs a
    regex.  Running it alone is how the reach of this engine gets
    measured without spending a budget per function to find out.
    """
    src = SRC / t["file"]
    rec = {"file": t["file"], "function": t["function"]}
    try:
        text = src.read_text(errors="replace")
    except OSError as e:
        return {**rec, "status": "ERROR", "detail": str(e)}
    params = parse_params(text, t["function"])
    if params is None:
        return {**rec, "status": "NOFUNC",
                "detail": "no definition of %s in this translation unit"
                          % t["function"]}
    if isinstance(params, str):
        return {**rec, "status": "ERROR", "detail": params}
    return {**rec, "status": "RUNNABLE", "nparams": len(params),
            "bytes": layout(params)[1]}


def _run_task(t: dict) -> dict:
    """One task in a worker.  Each gets its own workdir tree.

    afl-fuzz writes into -o and refuses to share it, so the directories
    have to be disjoint; the harness/corpus tree is per (file, function)
    already, and the parent is per worker here so two workers cannot
    collide on the same stem.
    """
    work = Path(tempfile.mkdtemp(prefix="fusebmc-w"))
    try:
        return fuzz_one(SRC / t["file"], t["function"], t["budget"], work,
                        t["cbmc_timeout"])
    except Exception as e:                     # a worker must not vanish
        return {"file": t["file"], "function": t["function"],
                "status": "ERROR", "detail": "driver: %r" % (e,)}
    finally:
        shutil.rmtree(work, ignore_errors=True)


def _done_pairs(out: Path) -> set:
    """(file, function) already in --out, for --resume."""
    seen = set()
    if not out.is_file():
        return seen
    for line in out.read_text(errors="replace").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            r = json.loads(line)
        except ValueError:
            continue
        if "function" in r:
            seen.add((r.get("file"), r["function"]))
    return seen


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", action="append", default=[],
                    help="path under hbsd/src, repeatable; pairs with "
                         "--function.  Omit both to work from --plan.")
    ap.add_argument("--function", action="append", default=[])
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"),
                    help="the port ledger, which says what each file defines")
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix under hbsd/src, repeatable")
    ap.add_argument("--limit", type=int,
                    help="stop after this many (file, function) pairs")
    ap.add_argument("--jobs", type=int, default=1,
                    help="functions to fuzz at once.  Each burns --budget "
                         "seconds of wall clock, so this is the only knob "
                         "that makes a whole scope finish.")
    ap.add_argument("--resume", action="store_true",
                    help="append to --out, skipping pairs already in it")
    ap.add_argument("--dry-run", action="store_true",
                    help="classify only: what this engine can and cannot "
                         "reach, without spending a budget to find out")
    ap.add_argument("--budget", type=int, default=30,
                    help="seconds of fuzzing per function")
    ap.add_argument("--cbmc-timeout", type=int, default=30)
    ap.add_argument("--out", default="fusebmc_results.jsonl")
    ap.add_argument("--keep", action="store_true",
                    help="keep the harness/corpus tree for inspection")
    args = ap.parse_args()

    if args.src or args.function:
        if len(args.src) != len(args.function):
            print("--src and --function must pair up", file=sys.stderr)
            return 2
        tasks = [{"file": s, "function": f}
                 for s, f in zip(args.src, args.function)]
    else:
        tasks = load_tasks(Path(args.plan), args.scope, args.limit)
        print("%d (translation unit, function) pair(s) from the ledger"
              % len(tasks), flush=True)

    out = Path(args.out)
    if args.resume:
        done = _done_pairs(out)
        before = len(tasks)
        tasks = [t for t in tasks if (t["file"], t["function"]) not in done]
        print("  resume: %d already recorded, %d to do"
              % (before - len(tasks), len(tasks)), flush=True)

    counts: dict[str, int] = {}
    reasons: dict[str, int] = {}

    def note(r: dict) -> None:
        counts[r["status"]] = counts.get(r["status"], 0) + 1
        if r["status"] == "ERROR" and r.get("detail"):
            # The reason is the measurement, so bucket on the REASON and
            # not on the text it quotes.  Keying on the raw detail put
            # `a pointer parameter (const char *s)' and
            # `a pointer parameter (struct foo *f)' in different buckets,
            # which turns one answer into forty and hides the shape.
            key = r["detail"]
            if "(" in key and ")" in key:      # `(void (*f)(int))' nests
                key = key[:key.index("(")] + key[key.rindex(")") + 1:]
            key = re.sub(r"\s+", " ", key).strip()[:60]
            reasons[key] = reasons.get(key, 0) + 1

    mode = "a" if (args.resume and out.is_file()) else "w"
    with open(out, mode) as fh:
        if mode == "w":
            fh.write(json.dumps({"_meta": "fusebmc", "budget": args.budget,
                                 "dry_run": bool(args.dry_run)}) + "\n")
        if args.dry_run:
            for t in tasks:
                r = classify_one(t)
                note(r)
                fh.write(json.dumps(r) + "\n")
        elif args.jobs > 1:
            from concurrent.futures import ProcessPoolExecutor, as_completed
            for t in tasks:
                t["budget"], t["cbmc_timeout"] = args.budget, args.cbmc_timeout
            with ProcessPoolExecutor(args.jobs) as ex:
                futs = {ex.submit(_run_task, t): t for t in tasks}
                for i, fu in enumerate(as_completed(futs), 1):
                    r = fu.result()
                    note(r)
                    print("[%d/%d] %-9s %s %s" % (i, len(tasks), r["status"],
                                                  r["file"], r["function"]),
                          flush=True)
                    fh.write(json.dumps(r) + "\n")
                    fh.flush()
        else:
            work = Path(tempfile.mkdtemp(prefix="fusebmc-"))
            if args.keep:
                print("workdir: %s" % work, file=sys.stderr)
            for t in tasks:
                r = fuzz_one(SRC / t["file"], t["function"], args.budget,
                             work, args.cbmc_timeout)
                note(r)
                print("%-9s %s %s%s" % (r["status"], t["file"], t["function"],
                                        ("  " + r["detail"][:70])
                                        if r.get("detail") else ""),
                      flush=True)
                fh.write(json.dumps(r) + "\n")
                fh.flush()
            if not args.keep:
                shutil.rmtree(work, ignore_errors=True)

    print("\n" + "  ".join("%s=%d" % kv for kv in sorted(counts.items())),
          file=sys.stderr)
    if reasons:
        print("\nwhy ERROR (top 10):", file=sys.stderr)
        for k, n in sorted(reasons.items(), key=lambda kv: -kv[1])[:10]:
            print("  %5d  %s" % (n, k), file=sys.stderr)
    print("\nCLEAN is not a proof. ERROR is the measurement of what this "
          "could not reach.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
