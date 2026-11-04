#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Does sys/sys/atomic_generic.h actually produce lock-free atomics?

The generic header is only worth having if the compiler lowers every
operation to instructions. Where it cannot, it emits a call to libatomic -
__atomic_fetch_add_8 and friends - which takes a lock in userland and does
not exist in the kernel at all. So "it compiles" is not the question. The
question is per (architecture, width), and this answers it by compiling and
looking at what came out.

Method, with no FreeBSD needed:

  * a shim <sys/cdefs.h> and <sys/types.h> so the header can be compiled
    with -nostdinc and nothing else on the include path - the header is
    then isolated, and a result cannot come from some other header;
  * a driver with one external-linkage wrapper per generated function, so
    the static inlines are actually emitted;
  * one object per (target, width), so an out-of-line call pins the width
    that could not be done in hardware;
  * llvm-nm on the object: any undefined __atomic_* symbol is a libcall.

Exit status is 1 if a width that the architecture is expected to do
lock-free is not, and 0 otherwise - including when a width genuinely cannot
be lock-free on that hardware, which is reported rather than failed.
"""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "hbsd/src/sys/sys/atomic_generic.h"

# The six PBSD builds, as clang spells them.
TARGETS = {
    "amd64":   "x86_64-unknown-freebsd15.1",
    "arm64":   "aarch64-unknown-freebsd15.1",
    "arm":     "armv7-unknown-freebsd15.1-eabihf",
    "i386":    "i386-unknown-freebsd15.1",
    "powerpc": "powerpc64-unknown-freebsd15.1",
    "riscv":   "riscv64-unknown-freebsd15.1",
}

# Width in bits -> is it required to be lock-free?
#
# A 32-bit machine that cannot do a 64-bit atomic in one instruction is not
# broken, it is 32-bit. armv7 has LDREXD/STREXD and i386 has CMPXCHG8B, so
# both can in principle; whether clang uses them without being told is
# exactly what this measures, so nothing is assumed here.
REQUIRED_WIDTHS = {
    "amd64":   (8, 16, 32, 64),
    "arm64":   (8, 16, 32, 64),
    "arm":     (8, 16, 32),
    "i386":    (8, 16, 32),
    "powerpc": (8, 16, 32, 64),
    "riscv":   (32, 64),
}

CDEFS_SHIM = """
#ifndef _SYS_CDEFS_H_
#define _SYS_CDEFS_H_
#define __inline        inline __attribute__((__always_inline__))
#define __CONCAT1(x,y)  x ## y
#define __CONCAT(x,y)   __CONCAT1(x,y)
#endif
"""

TYPES_SHIM = """
#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
#ifdef __LP64__
typedef unsigned long      uint64_t;
typedef unsigned long      uintptr_t;
#else
typedef unsigned long long uint64_t;
typedef unsigned int       uintptr_t;
#endif
typedef unsigned char      u_char;
typedef unsigned short     u_short;
typedef unsigned int       u_int;
typedef unsigned long      u_long;
#endif
"""

# (name, kind) for every operation the header generates at a fixed width.
# kind says how to call it, nothing more.
VOID_OPS = ["add", "subtract", "set", "clear"]
VOID_SFX = ["", "_acq", "_rel", "_barr"]
FETCH_OPS = ["fetchadd"]
FETCH_SFX = ["", "_acq", "_rel"]
CMPSET_SFX = ["", "_acq", "_rel"]
BIT_SFX = ["", "_acq"]


def driver_for_width(w: int) -> str:
    t = f"uint{w}_t"
    out = ["#include <sys/atomic_generic.h>", ""]
    n = 0

    def emit(sig: str) -> None:
        nonlocal n
        out.append(sig)
        n += 1

    for op in VOID_OPS:
        for sfx in VOID_SFX:
            emit(f"void w_{op}{sfx}_{w}(volatile {t} *p, {t} v)"
                 f" {{ atomic_{op}{sfx}_{w}(p, v); }}")
    for op in FETCH_OPS:
        for sfx in FETCH_SFX:
            emit(f"{t} w_{op}{sfx}_{w}(volatile {t} *p, {t} v)"
                 f" {{ return atomic_{op}{sfx}_{w}(p, v); }}")
    for sfx in CMPSET_SFX:
        emit(f"int w_cmpset{sfx}_{w}(volatile {t} *p, {t} a, {t} b)"
             f" {{ return atomic_cmpset{sfx}_{w}(p, a, b); }}")
        emit(f"int w_fcmpset{sfx}_{w}(volatile {t} *p, {t} *a, {t} b)"
             f" {{ return atomic_fcmpset{sfx}_{w}(p, a, b); }}")
    for sfx in BIT_SFX:
        emit(f"int w_tas{sfx}_{w}(volatile {t} *p, u_int b)"
             f" {{ return atomic_testandset{sfx}_{w}(p, b); }}")
    emit(f"int w_tac_{w}(volatile {t} *p, u_int b)"
         f" {{ return atomic_testandclear_{w}(p, b); }}")
    emit(f"{t} w_load_{w}(const volatile {t} *p)"
         f" {{ return atomic_load_{w}(p); }}")
    emit(f"{t} w_load_acq_{w}(const volatile {t} *p)"
         f" {{ return atomic_load_acq_{w}(p); }}")
    emit(f"void w_store_{w}(volatile {t} *p, {t} v)"
         f" {{ atomic_store_{w}(p, v); }}")
    emit(f"void w_store_rel_{w}(volatile {t} *p, {t} v)"
         f" {{ atomic_store_rel_{w}(p, v); }}")
    emit(f"{t} w_swap_{w}(volatile {t} *p, {t} v)"
         f" {{ return atomic_swap_{w}(p, v); }}")
    emit(f"{t} w_readandclear_{w}(volatile {t} *p)"
         f" {{ return atomic_readandclear_{w}(p); }}")
    out.append(f"/* {n} wrappers */")
    return "\n".join(out) + "\n", n


CTYPE_DRIVER = """
#include <sys/atomic_generic.h>

/* One call per C-type spelling, to prove the alias block instantiates. */
void  c_add_char(volatile u_char *p, u_char v)   { atomic_add_char(p, v); }
void  c_add_short(volatile u_short *p, u_short v){ atomic_add_short(p, v); }
void  c_add_int(volatile u_int *p, u_int v)      { atomic_add_int(p, v); }
void  c_add_long(volatile u_long *p, u_long v)   { atomic_add_long(p, v); }
void  c_add_ptr(volatile uintptr_t *p, uintptr_t v) { atomic_add_ptr(p, v); }
int   c_cmpset_long(volatile u_long *p, u_long a, u_long b)
      { return atomic_cmpset_long(p, a, b); }
int   c_fcmpset_ptr(volatile uintptr_t *p, uintptr_t *a, uintptr_t b)
      { return atomic_fcmpset_ptr(p, a, b); }
u_long c_load_acq_long(const volatile u_long *p)
      { return atomic_load_acq_long(p); }
void  c_store_rel_ptr(volatile uintptr_t *p, uintptr_t v)
      { atomic_store_rel_ptr(p, v); }
u_int c_swap_int(volatile u_int *p, u_int v) { return atomic_swap_int(p, v); }
int   c_tas_long(volatile u_long *p, u_int b)
      { return atomic_testandset_long(p, b); }
void  c_fences(void)
{
      atomic_thread_fence_acq();
      atomic_thread_fence_rel();
      atomic_thread_fence_acq_rel();
      atomic_thread_fence_seq_cst();
}
"""


def libcalls(obj: Path, nm: str) -> set[str]:
    out = subprocess.run([nm, "-u", str(obj)], capture_output=True, text=True)
    syms = set()
    for line in out.stdout.splitlines():
        sym = line.split()[-1]
        if sym.startswith("__atomic_") or sym.startswith("__sync_"):
            syms.add(sym)
    return syms


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cc", default="clang")
    ap.add_argument("--nm", default="llvm-nm")
    args = ap.parse_args()

    if not HEADER.is_file():
        print(f"FAIL {HEADER} is not here")
        return 1
    for tool in (args.cc, args.nm):
        if shutil.which(tool) is None:
            print(f"SKIP {tool} not installed; nothing measured")
            return 0

    tmp = Path(tempfile.mkdtemp(prefix="atomicgen."))
    inc = tmp / "sys"
    inc.mkdir()
    (inc / "cdefs.h").write_text(CDEFS_SHIM)
    (inc / "types.h").write_text(TYPES_SHIM)
    shutil.copy(HEADER, inc / "atomic_generic.h")

    widths = (8, 16, 32, 64)
    wrappers = {}
    for w in widths:
        src, n = driver_for_width(w)
        (tmp / f"w{w}.c").write_text(src)
        wrappers[w] = n
    (tmp / "ctype.c").write_text(CTYPE_DRIVER)

    print(f"sys/sys/atomic_generic.h — {sum(wrappers.values())} operations "
          f"per width set, {len(TARGETS)} targets\n")

    failed = False
    rows = []
    for arch, triple in TARGETS.items():
        cells = []
        for w in widths:
            obj = tmp / f"{arch}.{w}.o"
            cmd = [args.cc, "-target", triple, "-O2", "-ffreestanding",
                   "-nostdinc", f"-I{tmp}", "-std=gnu17", "-Wall",
                   "-c", str(tmp / f"w{w}.c"), "-o", str(obj)]
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode != 0:
                cells.append("ERR")
                print(f"FAIL {arch} width {w} did not compile:")
                print("     " + (r.stderr.strip().splitlines() or ["?"])[0])
                failed = True
                continue
            calls = libcalls(obj, args.nm)
            if not calls:
                cells.append("lock-free")
            else:
                cells.append("libcall")
                if w in REQUIRED_WIDTHS[arch]:
                    failed = True
                    print(f"FAIL {arch} width {w} is not lock-free: "
                          f"{', '.join(sorted(calls))}")
        # the C-type block, once per target
        obj = tmp / f"{arch}.ctype.o"
        cmd = [args.cc, "-target", triple, "-O2", "-ffreestanding",
               "-nostdinc", f"-I{tmp}", "-std=gnu17", "-Wall",
               "-c", str(tmp / "ctype.c"), "-o", str(obj)]
        r = subprocess.run(cmd, capture_output=True, text=True)
        ctype = "ok" if r.returncode == 0 else "ERR"
        if r.returncode != 0:
            failed = True
            print(f"FAIL {arch} C-type aliases did not compile:")
            print("     " + (r.stderr.strip().splitlines() or ["?"])[0])
        rows.append((arch, cells, ctype))

    print(f"{'arch':9s} {'8':>10s} {'16':>10s} {'32':>10s} {'64':>10s}   "
          f"char/short/int/long/ptr")
    for arch, cells, ctype in rows:
        print(f"{arch:9s} " + " ".join(f"{c:>10s}" for c in cells)
              + f"   {ctype}")

    print()
    if failed:
        print("A width an architecture is expected to do in hardware came out")
        print("as a call into libatomic. In the kernel that symbol does not")
        print("exist, so this is a link error waiting rather than a slow path.")
        return 1
    print("Every required width is lock-free on every target, and the C-type")
    print("spellings instantiate. A 'libcall' above is a width the hardware")
    print("does not have and the header does not pretend it does.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
