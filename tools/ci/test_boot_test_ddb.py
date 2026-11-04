#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Exercise boot_test.py's kernel-debugger probe against fake consoles.

Four times this session a console change was written, reasoned about, and
dispatched, and four times a fifty-minute run came back saying the code had
not done what it looked like it did: the loader menu pattern that matched
nothing because hotkeys are highlighted inside words, the fallback whose
loop body only ran when a chunk arrived, the forty-character line that
overran a sixteen-byte FIFO, and the GPT layout that was not the layout.

So the probe is exercised here first. The fakes are processes that speak
like the thing they stand in for, and each one asserts a different outcome:

  ddb        answers the break sequence with a db> prompt and real-shaped
             `ps` and `bt` output. The probe must reach it and record both.
  pager      the same, but pages at --More-- like db_output.c:257 does.
             The probe must answer the pager and still get the whole thing.
  deaf       ignores the break sequence entirely, as a kernel too wedged to
             take a trap would. The probe must give up and say so, not hang.

Run: python3 tools/ci/test_boot_test_ddb.py
"""

from __future__ import annotations

import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import boot_test  # noqa: E402


PS_OUTPUT = """  PID PPID PGRP  RGID   Flags Stat Wchan    Name
    1    0    1     0 0004002 [SLPQ wait 0xfffff8000a2b1000][SLP] init
    0    0    0     0 0000200 [SLPQ swapin 0xffffffff81b8f2c8][SLP] kernel
"""

BT_OUTPUT = """Tracing pid 1 tid 100002 td 0xfffff8000a2b0000
kdb_alt_break_internal() at kdb_alt_break_internal+0x15d
uart_intr() at uart_intr+0x148
--- interrupt, rip = RIPMARK, rsp = 0x67061406eb30 ---
"""


FAKE = r'''
import sys, os
mode = sys.argv[1]
out = sys.stdout.buffer
def w(b):
    out.write(b); out.flush()

w(b"start_init: trying /sbin/init\n")

state = []          # the CR ~ ^B state machine, as subr_kdb.c has it
entered = False
round_n = 1
buf = b""
while True:
    c = sys.stdin.buffer.read(1)
    if not c:
        break
    if not entered:
        if mode == "deaf":
            continue                      # a kernel that never answers
        if c == b"\r":
            state = [b"\r"]
        elif state == [b"\r"] and c == b"~":
            state = [b"\r", b"~"]
        elif state == [b"\r", b"~"] and c == b"\x02":
            entered = True
            w(b"\nKDB: enter: Break to debugger\n[ thread pid 0 tid 100000 ]\n"
              b"Stopped at      kdb_enter+0x32: movq    $0,0x1284a3(%rip)\ndb> ")
        else:
            state = []
        continue
    # at the db> prompt
    if c == b" " and buf == b"":
        # a page turn, answered by the pager branch
        w(PAGE2); w(b"db> "); continue
    buf += c
    if c in (b"\n", b"\r"):
        cmd = buf.strip().decode()
        buf = b""
        if cmd == "ps":
            if mode == "pager":
                w(PS1 + b"--More--\r")
            else:
                w(PS.encode() + b"db> ")
        elif cmd == "bt":
            w(BT.encode().replace(b"RIPMARK",
                                  b"0x2c2994c9fd%d" % round_n) + b"db> ")
        elif cmd == "c":
            entered = False; state = []; round_n += 1
        elif cmd == "":
            w(b"db> ")
        else:
            w(b"Unknown command \"" + cmd.encode() + b"\"\ndb> ")
'''


def run_fake(mode, cmds, timeout=8.0, rounds=1, gap=0.2):
    src = (FAKE
           .replace("PS1", repr(PS_OUTPUT.split("\n")[0].encode() + b"\n"))
           .replace("PAGE2",
                    repr("\n".join(PS_OUTPUT.split("\n")[1:]).encode()))
           .replace("PS", repr(PS_OUTPUT))
           .replace("BT", repr(BT_OUTPUT)))
    proc = subprocess.Popen([sys.executable, "-c", src, mode],
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, bufsize=0)
    # As main() does at boot_test.py:569. _drain() calls stdout.read() with
    # no select, so on a blocking pipe a guest that says nothing blocks the
    # reader forever and no deadline is ever reconsidered. The first version
    # of this harness left it blocking and the `deaf` fake hung the test -
    # which is the harness being wrong, not the probe, and is exactly the
    # kind of thing that has cost this project fifty-minute runs.
    os.set_blocking(proc.stdout.fileno(), False)
    log = open(os.devnull, "wb")
    boot_test.DDB_REPORT.clear()
    try:
        # let the fake print its first line
        time.sleep(0.2)
        ok = boot_test.break_to_ddb(proc, log, timeout, cmds,
                                    rounds=rounds, gap=gap)
    finally:
        proc.kill()
        log.close()
    return ok, list(boot_test.DDB_REPORT)


def main() -> int:
    failures = []

    def check(label, cond, detail=""):
        print(f"  {'ok  ' if cond else 'FAIL'} {label}")
        if not cond:
            failures.append(f"{label}: {detail}")

    print("break sequence is the three bytes subr_kdb.c:327 names")
    # Read it out of the module rather than restating it here.
    import inspect
    src = inspect.getsource(boot_test.break_to_ddb)
    check("sends CR ~ ^B", '_send(proc, "\\r~\\x02")' in src, src[:200])

    print("\nfake: a kernel that answers")
    ok, rep = run_fake("ddb", ["ps", "bt"])
    check("reached db>", ok)
    check("recorded the break", any(c == "break sequence" and "reached" in o
                                    for c, o in rep))
    check("ps names process 1",
          any(c == "ps" and " 1 " in o and "init" in o for c, o in rep),
          repr(rep))
    # The shape run 31 actually produced: the interrupt path, and under it
    # a frame whose rip is a userland address. That frame is the whole
    # finding, so the test asserts on it rather than on any old backtrace.
    check("bt captured the userland interrupt frame",
          any(c == "bt" and "--- interrupt, rip = 0x" in o
              for c, o in rep), repr(rep))

    print("\nfake: the same, paging at --More-- like db_output.c:257")
    ok, rep = run_fake("pager", ["ps"])
    check("reached db>", ok)
    check("got the whole ps through the pager",
          any(c == "ps" and "init" in o and "kernel" in o for c, o in rep),
          repr(rep))

    print("\nfake: a kernel too wedged to answer the break")
    t0 = time.time()
    ok, rep = run_fake("deaf", ["ps"], timeout=2.0)
    took = time.time() - t0
    check("gave up rather than hanging", not ok)
    check("gave up inside the timeout", took < 6.0, f"{took:.1f}s")
    check("said why",
          any("no db> prompt" in o for _, o in rep), repr(rep))
    check("did not invent command output",
          not any(c == "ps" for c, _ in rep), repr(rep))

    print("\nfake: two rounds, continuing in between")
    ok, rep = run_fake("ddb", ["bt"], rounds=2, gap=0.2)
    check("reached db> both times", ok)
    breaks = [c for c, _ in rep if c.startswith("break sequence")]
    check("recorded two breaks", len(breaks) == 2, repr(breaks))
    rips = [o for c, o in rep if c.startswith("bt")]
    check("captured two backtraces", len(rips) == 2, repr(rips))
    check("and they carry different rip values",
          len(rips) == 2 and rips[0] != rips[1], repr(rips))

    print()
    if failures:
        print(f"{len(failures)} FAILED")
        for f in failures:
            print(f"  {f}")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
