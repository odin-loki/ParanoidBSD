#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The debugger output has to be READ, not just printed.

Boot image run 43 printed a backtrace whose interrupt frame carried a USER
rip and a `show procvm 1` that placed that rip inside the second and last
executable mapping of pid 1 - the run-time linker, with no shared library
mapped after it - and then concluded

    FAIL the kernel started and never reached userland in 480s.

which its own output disproves. Same shape as run 40, where the correct
verdict was on screen and the wrong thing was read, and that cost six runs
at fifty minutes each.

So the reading is code, and this is run 43's real output fed back through
it.
"""
from __future__ import annotations
import io, sys
from contextlib import redirect_stdout
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from boot_test import _explain_ddb  # noqa: E402

# Trimmed from out/boot.log of run 43 (34011290743). Two rounds, and the
# rip and rsp are identical in both - which is the part that says "spinning
# at one instruction" rather than "making slow progress".
BT = """Tracing pid 1 tid 100002 td 0xfffff80004c98000
kdb_alt_break_internal() at kdb_alt_break_internal+0x15d/frame 0xfffffe0053c9ae10
uart_intr() at uart_intr+0x148/frame 0xfffffe0053c9ae90
Xapic_isr1() at Xapic_isr1+0xdc/frame 0xfffffe0053c9af30
--- interrupt, rip = 0x3d6201b9fd0, rsp = 0x6ab2caaa2f60, rbp = 0x6ab2caaa2f60 ---
"""

PROCVM = """p = 0xfffffe0051002190, vmspace = 0xfffff80004ca44e0, map = 0xfffff80004ca44e0
Task map 0xfffff80004ca44e0: pmap=0xfffff80004ca4618, nentries=12, version=16
  map entry 0xfffff800038ca8a0: start=0x6054539000, end=0x605453c000, eflags=0xc0c,
   prot=1/1/copy, object=0xfffff800040071e0, offset=0x0, copy (needed)
    Object 0xfffff800040071e0: type=2, size=0x8, res=8, ref=3, flags=0x1000
  map entry 0xfffff800038ca840: start=0x605453c000, end=0x6054541000, eflags=0xc0c,
   prot=5/5/copy, object=0xfffff800040071e0, offset=0x2000, copy (needed)
  map entry 0xfffff800038ca7e0: start=0x6054541000, end=0x6054542000, eflags=0x80c,
   prot=3/3/copy, ruid 0, object=0xfffff800040071e0, offset=0x6000, copy (needed)
  map entry 0xfffff800038ca780: start=0x6054542000, end=0x6054544000, eflags=0,
   prot=3/7/copy, object=0xfffff800040070f0, offset=0x0, obj ruid 0
  map entry 0xfffff800038ca720: start=0x3d6201b4000, end=0x3d6201b9000, eflags=0xc0c,
   prot=1/1/copy, object=0xfffff80004006e10, offset=0x0, copy (needed)
  map entry 0xfffff800038ca6c0: start=0x3d6201b9000, end=0x3d6201cd000, eflags=0xc0c,
   prot=5/5/copy, object=0xfffff80004006e10, offset=0x4000, copy (needed)
  map entry 0xfffff800038ca660: start=0x3d6201cd000, end=0x3d6201ce000, eflags=0,
   prot=3/7/copy, object=0xfffff80004006d20, offset=0x0, obj ruid 0
  map entry 0xfffff800038ca600: start=0x3d6201ce000, end=0x3d6201cf000, eflags=0x80c,
   prot=3/3/copy, ruid 0, object=0xfffff80004006e10, offset=0x17000, copy (needed)
  map entry 0xfffff800038ca5a0: start=0x3d6201cf000, end=0x3d6201d0000, eflags=0,
   prot=3/7/copy, object=0xfffff80004006c30, offset=0x0, obj ruid 0
  map entry 0xfffff800038ca4e0: start=0x6ab28aaa4000, end=0x6ab2caa84000, eflags=0x30000,
   prot=0/0/copy, object=0, offset=0x30003
  map entry 0xfffff800038ca540: start=0x6ab2caa84000, end=0x6ab2caaa4000, eflags=0x1000,
   prot=3/3/copy, object=0xfffff80004006b40, offset=0x0, obj ruid 0
  map entry 0xfffff800038ca900: start=0x7feabf695000, end=0x7feabf696000, eflags=0,
   prot=5/5/share, object=0xfffff80004007960, offset=0x0
"""

RUN43 = [("break sequence [1/2]", "db> reached"),
         ("bt 1 [1/2]", BT), ("bt [1/2]", BT), ("show procvm 1 [1/2]", PROCVM),
         ("break sequence [2/2]", "db> reached"),
         ("bt 1 [2/2]", BT), ("bt [2/2]", BT), ("show procvm 1 [2/2]", PROCVM)]

# A kernel rip, for the case the old message was actually right about.
KBT = """Tracing pid 1 tid 100002 td 0xfffff80004c98000
--- interrupt, rip = 0xffffffff80c1e2a0, rsp = 0xfffffe0053c9af38 ---
"""
KERNEL_CASE = [("bt 1", KBT)]

# A rip that moved between samples: running, not spinning.
MOVED = [("bt 1 [1/2]", BT),
         ("bt 1 [2/2]", BT.replace("rip = 0x3d6201b9fd0",
                                   "rip = 0x3d6201ba114"))]


def run(report):
    buf = io.StringIO()
    with redirect_stdout(buf):
        _explain_ddb(report)
    return buf.getvalue()


def main() -> int:
    fail = 0

    out = run(RUN43)
    print("== run 43's own output, read back")
    for line in out.splitlines():
        print("   " + line)
    checks = [
        ("USER address", "calls the rip a user address"),
        ("in USERLAND", "says pid 1 was in userland"),
        ("NOT a", "says it is not a kernel hang"),
        ("spinning at one instruction", "calls the identical samples a spin"),
        ("0xfd0 bytes into executable mapping #2 of 2",
         "locates the rip in the rtld, 0xfd0 in"),
        ("run-time linker", "names the run-time linker"),
        ("12 map entries", "counts the map entries"),
    ]
    for needle, what in checks:
        ok = needle in out
        print(f"  {'ok  ' if ok else 'FAIL'} {what}")
        fail += not ok

    # With a symbol table, the ELF vaddr becomes a name. 0x5fd0 is what
    # run 43's rip resolves to once the load base is taken off.
    import tempfile, os
    from boot_test import _load_symbols
    with tempfile.NamedTemporaryFile("w", suffix=".nm", delete=False) as f:
        f.write("0000000000005000 T _rtld_start\n"
                "0000000000005f80 T reloc_nonplt_self\n"
                "0000000000006200 T _rtld\n")
        nm = f.name
    buf = io.StringIO()
    with redirect_stdout(buf):
        _explain_ddb(RUN43, _load_symbols(nm))
    out = buf.getvalue()
    os.unlink(nm)
    for needle, what in [("ELF virtual address is 0x5fd0",
                          "computes the ELF vaddr from the load base"),
                         ("nearest preceding symbol: reloc_nonplt_self+0x50",
                          "names the nearest preceding symbol")]:
        ok = needle in out
        print(f"  {'ok  ' if ok else 'FAIL'} {what}")
        fail += not ok

    out = run(KERNEL_CASE)
    ok = "KERNEL address" in out and "USERLAND" not in out
    print(f"  {'ok  ' if ok else 'FAIL'} a kernel rip is still called a "
          "kernel rip")
    fail += not ok

    out = run(MOVED)
    ok = "not stuck at a single instruction" in out
    print(f"  {'ok  ' if ok else 'FAIL'} a rip that moved is not called a "
          "spin")
    fail += not ok

    ok = run([("bt", "no frames here")]) == ""
    print(f"  {'ok  ' if ok else 'FAIL'} says nothing when there is no "
          "interrupt frame")
    fail += not ok

    print("\n" + ("the debugger output is read correctly" if not fail
                  else f"FAILURES: {fail}"))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
