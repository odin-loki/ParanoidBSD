#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which kinds of defect the instruments can see, and which kinds nobody here can.

"We check for 90% of possible errors" is the sort of claim that is either
a measurement or a lie, and nothing in between. It is a measurement only
if three things are written down and can be argued with:

    the DENOMINATOR   which classes of defect were enumerated, so a
                      reader can point at one that is missing
    the MAPPING       which instrument sees which class, and how well
    the EXCLUSIONS    which classes are deliberately out of scope, named
                      rather than quietly dropped out of the denominator

That last one is where these numbers usually go wrong. A taxonomy that
omits data races and functional correctness scores much higher than one
that names them and admits nothing here checks them - and the codebase is
exactly as buggy either way. So the out-of-scope classes are in this
file, they are printed in every table, and they are counted in their own
denominator beside the headline rather than deleted from it.

Two coverage numbers, and they MULTIPLY
---------------------------------------
This file answers only the first of two questions:

    CLASS COVERAGE   of the kinds of defect that exist, how many does
                     some instrument here know how to look for?   <- this file
    CODE COVERAGE    of the code that exists, how much has actually been
                     put through those instruments?          <- confidence.py

An instrument set that can see every class of defect, run over 5% of the
tree, has found 5% of what is there. The product is the honest figure and
`confidence.py --taxonomy` prints it that way. Neither number alone means
anything, and quoting the class number by itself is the single easiest
way to mislead somebody with this file.

Strength, and what a silence is worth
-------------------------------------
    PROVES   a clean verdict DISCHARGES the class over all inputs within
             the stated bound. The instrument's silence is evidence.
             Only bounded model checking earns this, only for the
             properties it encodes, and only inside the unwind bound.
    FINDS    reports instances with useful recall. Its silence is NOT
             evidence: the class may be there and unreported.
    SOME     reports a recognisable subset only, and the subset is named
             in the note. Silence is worth nothing at all.
    (absent) cannot see the class.

A class counts as COVERED when some instrument reaches PROVES or FINDS.
PARTIAL means the best available is SOME. GAP means nothing sees it.
Those three are the only verdicts, and PARTIAL is not rounded up.

Availability
------------
`--available` re-runs the arithmetic over only the instruments actually
on PATH, which is nearly always a smaller number than the paper one and
is the only number that describes a real run. The difference between the
two is precisely the install list, and `--missing` prints it with the
package that supplies each one on FreeBSD and on Debian/Ubuntu.
"""
from __future__ import annotations

import argparse
import collections
import json
import shutil
import sys

# ---------------------------------------------------------------- strengths

PROVES = "PROVES"
FINDS = "FINDS"
SOME = "SOME"

_RANK = {PROVES: 3, FINDS: 2, SOME: 1}

COVERED = "COVERED"
PARTIAL = "PARTIAL"
GAP = "GAP"

# ---------------------------------------------------------------- instruments
#
# `probe` is what proves the instrument is really there. A name on PATH is
# not enough for the ones that are a Python driver in this tree, so those
# name their own file. `pkg`/`apt` are what the user installs at home; an
# instrument with no package is one this tree ships or one that has none.

INSTRUMENTS = {
    "cbmc": dict(
        bin="cbmc", pkg="cbmc", apt="cbmc",
        what="bounded model checker, per function, C",
        note="in this tree via tools/verify/cbmc_driver.py"),
    "esbmc": dict(
        bin="esbmc", pkg=None, apt=None,
        what="bounded model checker with k-induction, C and some C++",
        note="in no distribution package set; a release binary from "
             "github.com/esbmc/esbmc must be dropped on PATH by hand"),
    "fusebmc": dict(
        bin=None, driver="tools/verify/fusebmc.py", pkg=None, apt=None,
        what="test-generation-guided BMC: seeds from fuzzing, then proves",
        note="the published FuSeBMC is ESBMC+Map2Check+AFL and is not "
             "packaged anywhere; this tree implements the METHOD over the "
             "model checker it already has"),
    "clang-analyze": dict(
        bin="clang", pkg="llvm", apt="clang",
        what="path-sensitive symbolic execution, interprocedural within a TU",
        note="in this tree via tools/verify/analyze.py"),
    "clang-tidy": dict(
        bin="clang-tidy", pkg="llvm", apt="clang-tidy",
        what="AST matchers plus the analyzer's checks, C and C++"),
    "cppcheck": dict(
        bin="cppcheck", pkg="cppcheck", apt="cppcheck",
        what="flow-sensitive pattern analysis; its own value-flow engine",
        note="reaches classes clang's analyser does not, notably uninitialised "
             "struct members and some container misuse"),
    "coccinelle": dict(
        bin="spatch", pkg="coccinelle", apt="coccinelle",
        what="semantic patch matching over the C AST",
        note="the only instrument here that can be taught a defect SHAPE "
             "found by hand and then swept for it tree-wide"),
    "compiler-warnings": dict(
        bin="gcc", pkg="gcc", apt="gcc",
        what="gcc and clang diagnostics at -Wall -Wextra -Wconversion",
        note="two compilers disagree about which warnings they emit, so both "
             "are run and the union is taken"),
    "sanitizers": dict(
        bin="clang", pkg="llvm", apt="clang",
        what="UBSan, ASan, MSan, TSan - instrumentation that traps at RUNTIME",
        note="the only instrument here that needs the code to EXECUTE, so its "
             "reach is exactly the reach of the test corpus and no further; "
             "for kernel code that means almost nothing runs"),
    "pbsd-lints": dict(
        bin=None, driver="tools/verify/", pkg=None, apt=None,
        what="the bespoke lints in this tree, each written for a defect "
             "shape that had already been found by hand at least twice",
        note="lock_balance, onesided_index, realloc_self, capacity_first, "
             "nowait_check, noreturn_check, masked_switch_check, null_branch, "
             "sibling_guard and the per-subsystem probes"),
    "infer": dict(
        bin="infer", pkg=None, apt=None,
        what="separation-logic shape analysis, interprocedural across TUs",
        note="the only instrument in this list that reasons about the heap "
             "BETWEEN translation units; binary release from "
             "github.com/facebook/infer"),
    "codeql": dict(
        bin="codeql", pkg=None, apt=None,
        what="whole-program dataflow queries over a compiled database",
        note="free for open source; the taint-tracking queries are the part "
             "nothing else here replaces"),
}

# ------------------------------------------------------------------ classes


def C(cid, name, cwe, what, seen, note="", scope="in"):
    return dict(id=cid, name=name, cwe=cwe, what=what, seen=seen,
                note=note, scope=scope)


CLASSES = [

    # -- memory safety, spatial ------------------------------------------
    C("MEM-OOB-READ", "out-of-bounds read", [125],
      "a load through a subscript or pointer outside its object",
      {"cbmc": PROVES, "esbmc": PROVES, "fusebmc": FINDS,
       "clang-analyze": FINDS, "clang-tidy": SOME, "cppcheck": FINDS,
       "sanitizers": FINDS, "compiler-warnings": SOME, "infer": FINDS,
       "codeql": FINDS},
      "CBMC's --bounds-check discharges this for all inputs inside the "
      "unwind bound; that is the strongest statement anything here makes "
      "about any class."),

    C("MEM-OOB-WRITE", "out-of-bounds write", [787, 121, 122],
      "a store through a subscript or pointer outside its object",
      {"cbmc": PROVES, "esbmc": PROVES, "fusebmc": FINDS,
       "clang-analyze": FINDS, "clang-tidy": SOME, "cppcheck": FINDS,
       "sanitizers": FINDS, "compiler-warnings": SOME, "infer": FINDS,
       "codeql": FINDS},
      "same check as the read; separated because the consequence is not "
      "the same and the triage effort should not be either."),

    C("MEM-OOB-INDEX", "array index outside the array's range", [129],
      "a subscript that is bounded on one side and not the other, or not "
      "bounded at all",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": SOME,
       "cppcheck": FINDS, "pbsd-lints": FINDS, "coccinelle": FINDS,
       "sanitizers": FINDS, "codeql": FINDS},
      "onesided_index.py exists because this is the shape that kept "
      "recurring: `if (i > n) return' with no `i < 0'. CBMC proves it; "
      "the lint finds it in code CBMC cannot build."),

    C("MEM-PTR-ARITH", "pointer arithmetic leaving its object", [469, 823],
      "computing an address outside the object, whether or not it is "
      "then dereferenced",
      {"cbmc": PROVES, "esbmc": PROVES, "sanitizers": FINDS,
       "clang-analyze": SOME, "infer": SOME},
      "--pointer-overflow-check. UB on formation, not only on use, which "
      "is why it is its own class."),

    C("MEM-VLA-SIZE", "variable-length array of a bad size", [129, 789],
      "a VLA or alloca whose length is zero, negative or unbounded",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": FINDS,
       "sanitizers": FINDS, "cppcheck": SOME},
      "core.VLASize. alloca(0) returning a live pointer to nothing has "
      "already been found once in this tree."),

    # -- memory safety, temporal -----------------------------------------
    C("MEM-UAF", "use after free", [416],
      "a read or write through a pointer whose object has been released",
      {"clang-analyze": FINDS, "esbmc": PROVES, "cbmc": SOME,
       "sanitizers": FINDS, "cppcheck": FINDS, "infer": FINDS,
       "clang-tidy": SOME, "codeql": FINDS},
      "CBMC is SOME, not PROVES: it models the heap per function, so a "
      "free in one function and a use in its caller is invisible to a "
      "modular check. clang --analyze crosses that boundary inside a TU "
      "and is the instrument that actually finds these here."),

    C("MEM-DOUBLE-FREE", "double free", [415],
      "releasing the same object twice",
      {"clang-analyze": FINDS, "esbmc": PROVES, "cbmc": SOME,
       "sanitizers": FINDS, "cppcheck": FINDS, "infer": FINDS,
       "codeql": SOME},
      "unix.Malloc. Same modular limit as use-after-free."),

    C("MEM-LEAK", "allocation never released", [401],
      "an allocation whose last reference goes out of scope",
      {"clang-analyze": FINDS, "cbmc": SOME, "esbmc": FINDS,
       "sanitizers": FINDS, "cppcheck": FINDS, "infer": FINDS,
       "pbsd-lints": SOME},
      "CBMC's --memory-leak-check is per function and so reports a leak "
      "for every allocation the function hands back to its caller; it is "
      "SOME because the useful signal is swamped by that."),

    C("MEM-STACK-ESCAPE", "a pointer to a dead frame, escaped", [562],
      "returning or storing the address of a local past its lifetime",
      {"clang-analyze": FINDS, "compiler-warnings": SOME, "cbmc": SOME,
       "esbmc": FINDS, "sanitizers": FINDS, "cppcheck": FINDS},
      "core.StackAddressEscape. Found in this tree where a file-scope "
      "msghdr kept a pointer into a function's frame."),

    C("MEM-MISMATCHED-FREE", "released by the wrong deallocator", [762, 590],
      "free() of a non-heap or interior pointer; new[]/delete, "
      "malloc/delete and their mirrors",
      {"clang-analyze": FINDS, "clang-tidy": FINDS, "cppcheck": FINDS,
       "sanitizers": FINDS, "esbmc": FINDS},
      "unix.MismatchedDeallocator. The interior-pointer case - free() of "
      "&array[k] - has been found here by hand."),

    C("MEM-REALLOC-SELF", "realloc into its own pointer", [401],
      "`p = realloc(p, n)': on failure p is NULL and the old block is "
      "unreachable",
      {"pbsd-lints": FINDS, "coccinelle": FINDS, "clang-analyze": SOME,
       "cppcheck": SOME},
      "realloc_self.py. No general analyser reports this as a defect "
      "because it is not UB - it is a leak on a path that only runs when "
      "the machine is already out of memory, which is when it matters."),

    # -- pointer validity -------------------------------------------------
    C("PTR-NULL-DEREF", "null pointer dereference", [476],
      "a load or store through a pointer that can be null",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": FINDS,
       "fusebmc": FINDS, "cppcheck": FINDS, "sanitizers": FINDS,
       "infer": FINDS, "clang-tidy": SOME, "codeql": FINDS,
       "pbsd-lints": SOME},
      "CBMC proves it only where the pointer's provenance is inside the "
      "checked function. A parameter nobody constrains is reported as a "
      "failure and is a missing precondition, not a defect - which is "
      "what the report's triage buckets exist to separate."),

    C("PTR-UNCHECKED-ALLOC", "allocation result used without a null check",
      [690, 476],
      "malloc, M_NOWAIT, KM_NOSLEEP and friends, dereferenced unguarded",
      {"pbsd-lints": FINDS, "clang-analyze": FINDS, "coccinelle": FINDS,
       "cppcheck": FINDS, "cbmc": SOME},
      "nowait_check.py. The kernel half matters more than the userland "
      "half and no general tool knows that M_WAITOK cannot fail while "
      "M_NOWAIT can, so a general tool reports both or neither."),

    C("PTR-UNINIT", "dereference of an uninitialised pointer", [824],
      "a load through a pointer that was never assigned",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": FINDS,
       "compiler-warnings": SOME, "cppcheck": FINDS, "sanitizers": FINDS},
      ""),

    # -- uninitialised data ------------------------------------------------
    C("UNINIT-READ", "use of an uninitialised value", [457, 908],
      "reading a local or a struct member that no path assigned",
      {"clang-analyze": FINDS, "cbmc": SOME, "esbmc": FINDS,
       "compiler-warnings": SOME, "cppcheck": FINDS, "sanitizers": FINDS,
       "codeql": FINDS},
      "-Wmaybe-uninitialized is SOME and famously both misses and cries "
      "wolf; core.uninitialized.* is the instrument that carries this "
      "class here. MSan would be FINDS but needs the code to run."),

    C("UNINIT-BRANCH", "a branch decided by an uninitialised value", [457],
      "control flow taken on garbage",
      {"clang-analyze": FINDS, "cbmc": SOME, "esbmc": FINDS,
       "sanitizers": FINDS, "cppcheck": SOME},
      "core.uninitialized.Branch. Split from the plain read because the "
      "consequence is a wrong path, not a wrong value, and in kernel code "
      "that is usually the worse of the two."),

    C("UNINIT-RETURN", "a function returns an uninitialised value", [457],
      "a path that reaches the return without assigning what it returns",
      {"clang-analyze": FINDS, "compiler-warnings": SOME, "esbmc": FINDS,
       "cppcheck": FINDS},
      "core.uninitialized.UndefReturn. This tree has found the kernel "
      "copying such a value out to userspace."),

    C("INFOLEAK-PAD", "uninitialised bytes copied across a trust boundary",
      [200, 909, 226],
      "struct padding or a partially filled buffer handed to userspace, "
      "to a device, or onto the wire",
      {"pbsd-lints": SOME, "clang-analyze": SOME, "sanitizers": SOME,
       "codeql": SOME},
      "GAP IN PRACTICE. No instrument here models padding. The twelve "
      "bytes of kernel stack found in key_spdget were found by reading "
      "the code, and nothing would have reported it. This class is the "
      "clearest argument for adding a Coccinelle pattern library."),

    # -- integers ----------------------------------------------------------
    C("INT-SIGNED-OVF", "signed arithmetic overflow", [190],
      "a signed add, subtract, multiply or negate leaving the range - UB",
      {"cbmc": PROVES, "esbmc": PROVES, "fusebmc": FINDS,
       "sanitizers": FINDS, "clang-analyze": SOME, "cppcheck": SOME,
       "compiler-warnings": SOME},
      "--signed-overflow-check. The largest single source of verdicts in "
      "every sweep this tree has run."),

    C("INT-UNSIGNED-WRAP", "unsigned wraparound", [190, 191],
      "an unsigned computation wrapping; defined behaviour, often intended, "
      "sometimes a bug",
      {"cbmc": PROVES, "esbmc": PROVES, "sanitizers": FINDS,
       "cppcheck": SOME},
      "Not UB, so a report is a question and not a finding. Enabled here "
      "because the allocation-size case below is real and this is how it "
      "surfaces."),

    C("INT-TRUNC", "a narrowing conversion that loses value", [197, 681],
      "assigning a wider type to a narrower one where the value does not fit",
      {"cbmc": PROVES, "esbmc": PROVES, "compiler-warnings": FINDS,
       "clang-tidy": FINDS, "cppcheck": FINDS, "sanitizers": SOME},
      "-Wconversion is the cheapest instrument in this whole file and the "
      "noisiest; it is worth running exactly once per subsystem and "
      "triaging in bulk. CBMC's --conversion-check decides it per path."),

    C("INT-SIGN-CONV", "a signed/unsigned conversion that changes meaning",
      [195, 196],
      "a negative value becoming huge, or a comparison between the two",
      {"cbmc": PROVES, "esbmc": PROVES, "compiler-warnings": FINDS,
       "clang-tidy": FINDS, "cppcheck": FINDS},
      "-Wsign-compare and -Wsign-conversion. The mechanism behind most "
      "one-sided index bugs: `i > n' where i is signed and n unsigned."),

    C("INT-DIV-ZERO", "division or modulo by zero", [369],
      "a divisor that can be zero",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": FINDS,
       "fusebmc": FINDS, "sanitizers": FINDS, "cppcheck": FINDS,
       "pbsd-lints": SOME},
      "core.DivideZero and --div-by-zero-check. Two setsockopt calls "
      "giving an unprivileged user a kernel divide by zero were found "
      "here by exactly this pair."),

    C("INT-SHIFT-UB", "an undefined shift", [1335],
      "a shift count negative or at least the width, or 1<<31 into an int",
      {"cbmc": PROVES, "esbmc": PROVES, "sanitizers": FINDS,
       "clang-analyze": FINDS, "compiler-warnings": SOME, "cppcheck": FINDS},
      "--undefined-shift-check. `(uint32_t)x << 16' fixes found in the "
      "network drivers were all this class."),

    C("INT-ALLOC-SIZE", "an allocation size computed by overflow", [680, 131],
      "n * sizeof(x) wrapping, so the allocation is small and the writes "
      "are not",
      {"cbmc": PROVES, "esbmc": PROVES, "clang-analyze": SOME,
       "cppcheck": SOME, "codeql": FINDS, "coccinelle": FINDS},
      "The class where an integer bug becomes a heap overflow, which is "
      "why it is listed apart from INT-SIGNED-OVF."),

    C("FLOAT-UB", "float NaN, overflow and division", [369, 682],
      "a float computation producing NaN or inf where the caller assumes "
      "neither",
      {"cbmc": PROVES, "esbmc": PROVES, "sanitizers": SOME},
      "--nan-check and --float-overflow-check. Mostly relevant to msun "
      "and to the handful of kernel paths that touch the FPU."),

    # -- control flow and API contract ------------------------------------
    C("CTRL-MISSING-RETURN", "a path that falls out where it must return",
      [394, 758],
      "control reaching the end of a value-returning function",
      {"compiler-warnings": FINDS, "clang-analyze": FINDS, "cppcheck": FINDS,
       "clang-tidy": FINDS},
      "-Wreturn-type. One of the few classes a compiler covers outright."),

    C("CTRL-FALLTHROUGH", "an unintended switch fallthrough", [484],
      "a case running into the next with no annotation",
      {"compiler-warnings": FINDS, "clang-tidy": FINDS, "cppcheck": FINDS,
       "coccinelle": FINDS},
      "-Wimplicit-fallthrough. High false-positive rate on deliberate "
      "fallthrough in old code, which is why it is a one-time sweep."),

    C("CTRL-MASKED-SWITCH", "a switch over a mask with too few arms", [],
      "a bit mask with N reachable states dispatched by a switch with "
      "fewer than N arms and no default",
      {"pbsd-lints": FINDS},
      "masked_switch_check.py. NOT A KNOWN CWE and no general tool looks "
      "for it; it was written here after the shape appeared twice."),

    C("CTRL-DEAD-GUARD", "a guard that can never fire", [570, 571],
      "a bound check made vacuous by the type, so the check reads as "
      "protection and is not",
      {"compiler-warnings": SOME, "cbmc": SOME, "clang-analyze": SOME,
       "cppcheck": FINDS, "coccinelle": FINDS},
      "-Wtautological-compare catches the trivial half. `if (u < 0)' on "
      "an unsigned is the canonical instance and is exactly how a "
      "one-sided index bound is written by accident."),

    C("CTRL-SIBLING-ASYMMETRY", "one of a pair guards, the other does not", [],
      "two functions in a file take the same parameter and only one "
      "bounds it",
      {"pbsd-lints": FINDS},
      "sibling_guard.py. No CWE, no general tool. It found three real "
      "defects on its first run and its high tier measures 37% false "
      "positives, which is the honest cost of the class."),

    C("API-IGNORED-ERROR", "a failure return nobody looks at", [252, 391],
      "a call that reports failure through its return value, discarded",
      {"compiler-warnings": SOME, "clang-tidy": FINDS, "cppcheck": FINDS,
       "coccinelle": FINDS, "codeql": FINDS, "clang-analyze": SOME},
      "-Wunused-result only fires where the declaration carries "
      "warn_unused_result, which in this tree is almost nowhere. The "
      "sixteen unchecked iwn_read_prom_data calls were found by hand."),

    C("API-NORETURN", "an exiting helper not declared noreturn", [],
      "a function that always exits, whose callers then look to every "
      "analyser as if they continue",
      {"pbsd-lints": FINDS},
      "noreturn_check.py. This class matters mostly because it POISONS "
      "the others: an undeclared noreturn manufactures uninitialised-read "
      "and null-dereference reports on paths that do not exist."),

    C("API-PRECONDITION", "a documented contract enforced nowhere", [],
      "a range or nullness the comments state and no callee checks",
      {"cbmc": SOME, "esbmc": SOME, "pbsd-lints": SOME},
      "The isa_dma case: a channel contract written six times in comments "
      "and enforced in none of nine entry points. A model checker reports "
      "it as an unconstrained parameter, which is indistinguishable from "
      "a missing precondition until a person decides which it is."),

    # -- resources and locking ---------------------------------------------
    C("LOCK-LEAK", "a path that takes a lock and does not release it", [667],
      "an early return between acquire and release",
      {"pbsd-lints": FINDS, "coccinelle": FINDS, "clang-analyze": SOME,
       "infer": FINDS, "cppcheck": SOME},
      "lock_balance.py, written because the default clang checkers have "
      "no model for mtx_lock and so report nothing at all. Ten found on "
      "its first run."),

    C("LOCK-ORDER", "lock order inversion", [833],
      "two paths taking the same two locks in opposite orders",
      {"infer": SOME, "codeql": SOME},
      "GAP HERE. Nothing available in this container looks for it. "
      "WITNESS and the kernel's own lock-order checker find these at "
      "RUNTIME and that is currently the only instrument this project "
      "has for the class - which means it covers the paths a boot "
      "exercises and nothing else."),

    C("RES-FD-LEAK", "a descriptor or handle leaked", [775, 403],
      "an open with no close on some path",
      {"clang-analyze": SOME, "cppcheck": FINDS, "infer": FINDS,
       "coccinelle": FINDS},
      ""),

    C("RES-SLEEP-IN-ATOMIC", "a sleeping call under a spin lock", [],
      "M_WAITOK, or any sleeping primitive, inside a critical section",
      {"coccinelle": FINDS, "pbsd-lints": SOME},
      "Kernel-specific and invisible to every general tool, because "
      "nothing else knows which of this tree's thousands of functions "
      "can sleep. A Coccinelle rule plus a hand-built sleeper list is "
      "the only way to reach it."),

    # -- strings and formats -----------------------------------------------
    C("STR-FMT-NONLITERAL", "user data used as a format string", [134],
      "a printf-family format that is not a literal",
      {"compiler-warnings": FINDS, "clang-analyze": SOME, "clang-tidy": FINDS,
       "cppcheck": FINDS, "codeql": FINDS, "coccinelle": FINDS},
      "-Wformat-nonliteral -Wformat-security. mount_msdosfs passing its "
      "command line as a format string was found here."),

    C("STR-UNBOUNDED-COPY", "an unbounded copy into a fixed buffer", [120, 121],
      "strcpy, strcat, sprintf, gets into an array",
      {"cbmc": PROVES, "esbmc": PROVES, "compiler-warnings": SOME,
       "cppcheck": FINDS, "clang-analyze": SOME, "sanitizers": FINDS,
       "coccinelle": FINDS},
      "The bounds check catches it where the model builds. NOTE: this "
      "tree deliberately calls strcpy in places where the length is "
      "known, so a bare grep for the name is policy and not analysis - "
      "which is why analyze.py does not run security.insecureAPI.*."),

    C("STR-OFF-BY-ONE", "the terminator that does not fit", [193, 787],
      "a length computed without room for the NUL, or an inclusive bound "
      "written as exclusive",
      {"cbmc": PROVES, "esbmc": PROVES, "cppcheck": FINDS,
       "sanitizers": FINDS, "clang-analyze": SOME},
      ""),

    C("STR-NULL-ARG", "a string function handed null", [476],
      "strlen, strcpy, memcpy on a pointer that can be null",
      {"clang-analyze": FINDS, "cbmc": PROVES, "esbmc": PROVES,
       "cppcheck": FINDS},
      "unix.cstring.NullArg and unix.cstring.BadSizeArg."),

    # -- trust boundaries, kernel-specific ----------------------------------
    C("TRUST-UNVALIDATED-INPUT",
      "a value from outside the kernel used unchecked", [20, 129],
      "an ioctl field, a sysctl, a copyin length or a syscall argument "
      "reaching an index, a size or a divisor",
      {"cbmc": SOME, "esbmc": SOME, "codeql": FINDS, "pbsd-lints": SOME,
       "coccinelle": SOME, "clang-analyze": SOME},
      "THE MOST IMPORTANT CLASS IN THIS FILE AND THE WORST COVERED. A "
      "model checker treats every parameter as unconstrained, so it "
      "reports the attacker-controlled and the caller-constrained "
      "identically and a person must separate them - which is the single "
      "biggest cost in this whole project. Taint tracking is what "
      "actually answers it and codeql is the only instrument here that "
      "does taint, and it is not installed."),

    C("TRUST-UNCHECKED-DEVICE",
      "a value from a device, firmware or disk, trusted", [1285, 20],
      "a descriptor read off hardware, a length from an on-disk header, "
      "a field from a DMA'd structure",
      {"cbmc": SOME, "esbmc": SOME, "codeql": SOME, "pbsd-lints": SOME},
      "Same shape as the above and even less visible, because the "
      "source is a volatile read or a bus_space_read and no dataflow "
      "engine here is told those are untrusted."),

    C("TRUST-STACK-DISCLOSURE", "kernel stack copied to userspace",
      [200, 909],
      "a copyout of a structure some path did not fully fill",
      {"pbsd-lints": SOME},
      "See INFOLEAK-PAD. Effectively a GAP; every instance found in this "
      "tree was found by reading."),

    # -- C++ ----------------------------------------------------------------
    C("CXX-NEW-DELETE", "new/delete mismatched or leaked", [762, 401],
      "new[] freed with delete, new freed with free, or never freed",
      {"clang-analyze": FINDS, "clang-tidy": FINDS, "cppcheck": FINDS,
       "sanitizers": FINDS},
      "cplusplus.NewDelete and cplusplus.NewDeleteLeaks. Note that CBMC "
      "and ESBMC are absent from this row: neither handles real C++ at "
      "the level KDE is written in, which is why the C++ tier of this "
      "harness is clang-based and why its guarantees are weaker."),

    C("CXX-USE-AFTER-MOVE", "a moved-from object used", [416],
      "reading an object after std::move except to reassign or destroy",
      {"clang-tidy": FINDS, "clang-analyze": SOME},
      "bugprone-use-after-move. No bounded model checker here sees it."),

    C("CXX-DANGLING-REF", "a reference or view outliving its owner", [416, 562],
      "a reference, string_view, span or iterator into a destroyed or "
      "reallocated container",
      {"clang-tidy": SOME, "clang-analyze": SOME, "cppcheck": SOME,
       "sanitizers": FINDS, "compiler-warnings": SOME},
      "PARTIAL AND IT MATTERS. -Wdangling-gsl and the lifetime checkers "
      "catch the textbook cases and miss the ones that cross a function "
      "boundary. In a Qt codebase this is the commonest real defect and "
      "the honest verdict is that this harness will miss most of them."),

    C("CXX-ITERATOR-INVALID", "an iterator invalidated and then used", [416],
      "mutating a container while iterating it",
      {"clang-tidy": SOME, "cppcheck": FINDS, "sanitizers": SOME},
      ""),

    C("CXX-SELF-ASSIGN", "self-assignment corrupting the object", [],
      "an operator= that frees before it copies",
      {"clang-tidy": FINDS, "clang-analyze": SOME},
      ""),

    C("CXX-VIRTUAL-IN-CTOR", "a virtual call during construction", [],
      "dispatching to an override that runs before its object exists",
      {"clang-tidy": FINDS, "cppcheck": FINDS},
      ""),

    C("CXX-EXCEPTION-LEAK", "a throw between allocation and ownership",
      [401, 460],
      "a raw owning pointer held across a call that can throw",
      {"clang-tidy": SOME, "cppcheck": SOME, "infer": SOME},
      "PARTIAL. Requires knowing which calls throw, which in a Qt tree "
      "means knowing Qt, which none of these instruments do."),

    # -- out of scope, named rather than dropped -----------------------------
    C("CONC-DATA-RACE", "a data race", [362, 366],
      "two threads touching the same object, one writing, unsynchronised",
      {"sanitizers": SOME, "infer": SOME, "codeql": SOME},
      "OUT OF SCOPE. TSan is the real instrument and it needs the code to "
      "RUN under a workload that exercises the interleaving. For a kernel "
      "that means a booted system under load, not a static sweep. Nothing "
      "in this harness addresses it and no number in this project should "
      "be read as covering it.", scope="out"),

    C("CONC-TOCTOU", "check now, use later", [367],
      "a condition validated and then acted on after it can have changed",
      {"codeql": SOME, "coccinelle": SOME},
      "OUT OF SCOPE. The libusb refcount re-read after the lock was "
      "dropped is an instance found here by reading. No instrument "
      "available finds this class.", scope="out"),

    C("CONC-ATOMICITY", "a compound operation that is not atomic", [366, 662],
      "read-modify-write on shared state without the lock held throughout",
      {"infer": SOME},
      "OUT OF SCOPE, same reason.", scope="out"),

    C("LOGIC-WRONG-RESULT", "the function computes the wrong answer", [],
      "no undefined behaviour, no memory error, no crash - simply wrong",
      {},
      "OUT OF SCOPE AND THE LARGEST CLASS THERE IS. CBMC proving a "
      "function has no UB says nothing about whether it is correct; a "
      "driver that returns the wrong value for every input scores a "
      "clean sweep. Closing this needs a SPECIFICATION per function and "
      "this project has none. Everything else in this file is cheap by "
      "comparison.", scope="out"),

    C("SPEC-MISSING", "no specification exists to check against", [],
      "the class of defect that cannot even be stated because nobody "
      "wrote down what the code is supposed to do",
      {},
      "OUT OF SCOPE. Listed because it is the reason LOGIC-WRONG-RESULT "
      "is out of scope, and because a reader should see that the "
      "denominator of this whole file stops here.", scope="out"),

    C("CRYPTO-MISUSE", "a cryptographic primitive used wrongly", [327, 330],
      "a reused nonce, a weak PRNG, a non-constant-time comparison",
      {"codeql": SOME, "coccinelle": SOME},
      "OUT OF SCOPE. Needs a reviewer who knows the protocol.",
      scope="out"),

    C("SIDE-CHANNEL", "a timing or cache side channel", [208, 385],
      "a secret-dependent branch or memory access",
      {},
      "OUT OF SCOPE. No instrument here, and the ones that exist "
      "elsewhere need annotated secrets.", scope="out"),

    C("BUILD-CONFIG", "a defect only reachable under an option nobody builds",
      [1105],
      "code behind an #ifdef that no configuration in the sweep defines",
      {"pbsd-lints": SOME},
      "PARTIALLY IN SCOPE AND WORTH WATCHING. Every instrument here sees "
      "one preprocessor configuration per translation unit. This tree has "
      "already found that a wrong -include order compiled out every ZFS "
      "assertion, and that a module's dead OPT_FDT test hid a file - both "
      "were invisible defects in what the instruments could SEE, not in "
      "what they reported.", scope="out"),
]

# ------------------------------------------------------------------- queries


def verdict(cls: dict, tools: set[str] | None = None) -> str:
    """COVERED, PARTIAL or GAP, over `tools` (default: every instrument)."""
    best = 0
    for tool, strength in cls["seen"].items():
        if tools is not None and tool not in tools:
            continue
        best = max(best, _RANK.get(strength, 0))
    return {3: COVERED, 2: COVERED, 1: PARTIAL, 0: GAP}[best]


def available_tools() -> set[str]:
    """The instruments actually runnable here, checked rather than assumed."""
    out = set()
    for name, spec in INSTRUMENTS.items():
        exe = spec.get("bin")
        if exe and shutil.which(exe):
            out.add(name)
        elif spec.get("driver"):
            # Shipped by this tree; present because the tree is present.
            out.add(name)
    return out


def coverage(tools: set[str] | None = None) -> dict:
    """The headline arithmetic, with its denominator attached."""
    res = {"in": collections.Counter(), "out": collections.Counter()}
    for cls in CLASSES:
        res[cls["scope"]][verdict(cls, tools)] += 1
    inn = res["in"]
    total = sum(inn.values())
    return {
        "classes_in_scope": total,
        "covered": inn[COVERED],
        "partial": inn[PARTIAL],
        "gap": inn[GAP],
        "fraction_covered": (inn[COVERED] / total) if total else 0.0,
        "fraction_covered_or_partial":
            ((inn[COVERED] + inn[PARTIAL]) / total) if total else 0.0,
        "classes_out_of_scope": sum(res["out"].values()),
        "tools_counted": sorted(tools) if tools is not None else "all",
    }


# -------------------------------------------------------------------- output

def _fmt_seen(cls, tools):
    bits = []
    for tool, s in sorted(cls["seen"].items(),
                          key=lambda kv: (-_RANK.get(kv[1], 0), kv[0])):
        mark = "" if (tools is None or tool in tools) else "~"
        bits.append(f"{mark}{tool}:{s}")
    return " ".join(bits) or "-"


def print_table(tools, only_scope=None, gaps_only=False):
    for scope in ("in", "out"):
        if only_scope and scope != only_scope:
            continue
        head = ("IN SCOPE" if scope == "in" else
                "OUT OF SCOPE - named, not dropped")
        rows = [c for c in CLASSES if c["scope"] == scope]
        if gaps_only:
            rows = [c for c in rows if verdict(c, tools) != COVERED]
        if not rows:
            continue
        print(f"\n== {head}")
        for c in rows:
            v = verdict(c, tools)
            cwe = ",".join(f"CWE-{n}" for n in c["cwe"]) or "-"
            print(f"\n  [{v:7}] {c['id']}  {c['name']}")
            print(f"            {cwe}")
            print(f"            {c['what']}")
            print(f"            seen by: {_fmt_seen(c, tools)}")
            if c["note"]:
                for line in _wrap(c["note"], 66):
                    print(f"            {line}")


def _wrap(text, width):
    import textwrap
    return textwrap.wrap(text, width) or [""]


def print_coverage(cov, tools):
    print("\n== class coverage")
    print(f"  classes in scope          {cov['classes_in_scope']}")
    print(f"  COVERED (PROVES or FINDS) {cov['covered']}")
    print(f"  PARTIAL (SOME only)       {cov['partial']}")
    print(f"  GAP     (nothing sees it) {cov['gap']}")
    print(f"  classes named out of scope {cov['classes_out_of_scope']}")
    print(f"\n  fraction COVERED                {cov['fraction_covered']:.2f}")
    print("  fraction COVERED or PARTIAL     "
          f"{cov['fraction_covered_or_partial']:.2f}")
    if tools is not None:
        print(f"\n  counted over instruments present here: {len(tools)}")
        print(f"    {' '.join(sorted(tools))}")
    print("""
  This is CLASS coverage: of the kinds of defect enumerated, how many
  some instrument knows how to look for. It says NOTHING about how much
  code has been through those instruments. Multiply it by the code
  coverage confidence.py reports for the scope before quoting a figure.""")


def print_missing(tools):
    missing = [n for n in INSTRUMENTS if n not in tools]
    if not missing:
        print("every instrument this file knows about is present.")
        return
    print("\n== instruments not present here")
    would = coverage(set(INSTRUMENTS))
    have = coverage(tools)
    print(f"  with everything: {would['fraction_covered']:.2f} covered")
    print(f"  with what is here: {have['fraction_covered']:.2f} covered")
    for name in missing:
        spec = INSTRUMENTS[name]
        gained = [c["id"] for c in CLASSES
                  if c["scope"] == "in"
                  and verdict(c, tools) != COVERED
                  and verdict(c, tools | {name}) == COVERED]
        print(f"\n  {name}  -  {spec['what']}")
        if spec.get("pkg"):
            print(f"      FreeBSD:  pkg install {spec['pkg']}")
        if spec.get("apt"):
            print(f"      Debian:   apt install {spec['apt']}")
        if not spec.get("pkg") and not spec.get("apt"):
            print("      no package: see note")
        if spec.get("note"):
            for line in _wrap(spec["note"], 64):
                print(f"      {line}")
        if gained:
            print(f"      installing it would newly COVER: {' '.join(gained)}")
        else:
            print("      covers nothing that is not already covered")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--available", action="store_true",
                    help="count only instruments actually on PATH here")
    ap.add_argument("--table", action="store_true", help="the full matrix")
    ap.add_argument("--gaps", action="store_true",
                    help="only the classes not COVERED")
    ap.add_argument("--missing", action="store_true",
                    help="instruments absent, what they would buy, how to "
                         "install them")
    ap.add_argument("--tool", help="what one instrument covers")
    ap.add_argument("--cwe", type=int, help="which classes carry a CWE")
    ap.add_argument("--json", action="store_true",
                    help="machine-readable, for the orchestrator")
    args = ap.parse_args(argv)

    tools = available_tools() if args.available else None

    if args.json:
        json.dump({
            "instruments": INSTRUMENTS,
            "classes": [dict(c, verdict=verdict(c, tools)) for c in CLASSES],
            "coverage": coverage(tools),
            "available": sorted(available_tools()),
        }, sys.stdout, indent=1)
        print()
        return 0

    if args.tool:
        if args.tool not in INSTRUMENTS:
            print(f"no such instrument: {args.tool}", file=sys.stderr)
            print(f"known: {' '.join(sorted(INSTRUMENTS))}", file=sys.stderr)
            return 2
        spec = INSTRUMENTS[args.tool]
        print(f"{args.tool}  -  {spec['what']}")
        if spec.get("note"):
            for line in _wrap(spec["note"], 70):
                print(f"  {line}")
        print(f"  present here: "
              f"{'yes' if args.tool in available_tools() else 'NO'}")
        for strength in (PROVES, FINDS, SOME):
            hits = [c for c in CLASSES if c["seen"].get(args.tool) == strength]
            if hits:
                print(f"\n  {strength}:")
                for c in hits:
                    print(f"    {c['id']:28} {c['name']}")
        alone = [c["id"] for c in CLASSES
                 if c["scope"] == "in"
                 and verdict(c, set(INSTRUMENTS) - {args.tool}) != COVERED
                 and verdict(c) == COVERED]
        print(f"\n  classes NOTHING ELSE covers: {' '.join(alone) or '-'}")
        return 0

    if args.cwe:
        for c in CLASSES:
            if args.cwe in c["cwe"]:
                print(f"{verdict(c, tools):8} {c['id']:28} {c['name']}")
        return 0

    if args.missing:
        print_missing(available_tools())
        return 0

    if args.table or args.gaps:
        print_table(tools, gaps_only=args.gaps)

    print_coverage(coverage(tools), tools)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
