#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""interrogate() must capture a command's OUTPUT, not the shell's echo of it.

Boot run 59 was the first PBSD image with a login. It booted, logged in and
answered - and four of its five answer files contained the command instead
of the answer:

    uname.txt:  `; uname -a 2>&1; echo `

A serial console echoes what you type, so the line the shell shows back

    root@freebsd:~ # echo __PBSD_0_BEGIN__; uname -a 2>&1; echo __PBSD_0_END__

contains BOTH markers. The matcher found END there, took the text between
the two as the body, and wrote it out. Two markers instead of one does not
help: they are both on that line.

hardening.txt was the one that worked, and it worked by accident - its
echoed line was long enough to wrap at eighty columns and came back as
`echo __PBSD_1_END __', with a space, which did not match. So the capture
waited for the real output. One of five right, for a reason unconnected to
the code being right, which is the kind of pass that hides a defect for as
long as nobody looks at the other four.

The output markers start a line. The echo never does - a prompt and
`echo ' precede it. That is the fix, and this is the test for it.

Run: python3 tools/ci/test_interrogate_echo.py
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import boot_test as bt  # noqa: E402


# A console that behaves like the real one: it echoes the line it is given,
# then runs it. `slow' additionally takes its time, the way `find / -xdev'
# over a 7.5GB image does.
FAKE = r'''
import os, sys, time
mode = sys.argv[1]
sys.stdout.write("\r\nroot@freebsd:~ # ")
sys.stdout.flush()
for line in sys.stdin:
    line = line.rstrip("\n")
    if not line:
        continue
    # echo, exactly as a tty does
    sys.stdout.write(line + "\r\n")
    sys.stdout.flush()
    m = __import__("re").match(r"echo (\S+); (.*); echo (\S+)$", line)
    if not m:
        continue
    begin, cmd, end = m.groups()
    if mode == "slow" and "find" in cmd:
        time.sleep(2)
    sys.stdout.write(begin + "\r\n")
    sys.stdout.write("THE-REAL-OUTPUT-OF " + cmd.split(" 2>&1")[0] + "\r\n")
    sys.stdout.write(end + "\r\n")
    sys.stdout.write("root@freebsd:~ # ")
    sys.stdout.flush()
'''


def run_case(mode, commands, timeout):
    """Drive interrogate() against the fake console; return {name: text}."""
    with tempfile.TemporaryDirectory() as td:
        script = os.path.join(td, "fake.py")
        Path(script).write_text(FAKE)
        proc = subprocess.Popen(
            [sys.executable, "-u", script, mode],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT)
        # _drain() does a bare read() and relies on the caller having made
        # the pipe non-blocking, exactly as main() does at boot_test.py:950.
        # Without this the first read blocks for 65536 bytes that a fake
        # console will never produce, and the test hangs rather than fails.
        os.set_blocking(proc.stdout.fileno(), False)
        log = open(os.path.join(td, "boot.log"), "wb")
        try:
            ok, note = bt.interrogate(proc, log, commands, "root", "",
                                      timeout, td)
        finally:
            log.close()
            proc.kill()
            proc.wait()
        out = {}
        for name, _ in commands:
            p = os.path.join(td, f"{name}.txt")
            out[name] = Path(p).read_text() if os.path.exists(p) else None
        return ok, note, out


def main() -> int:
    failures = []

    def check(cond, what):
        print(("  ok   " if cond else "  FAIL ") + what)
        if not cond:
            failures.append(what)

    print("the echoed command line must not be mistaken for the output")
    ok, note, out = run_case("fast", [("uname", "uname -a"),
                                      ("hardening", "sysctl hardening")], 20)
    check(ok, f"interrogate() succeeded ({note})")
    for name in ("uname", "hardening"):
        got = (out[name] or "").strip()
        check(got.startswith("THE-REAL-OUTPUT-OF"),
              f"{name}.txt holds the output, not the echo (got {got!r:.60})")
        check("2>&1" not in got, f"{name}.txt is not the command line")

    print("\na slow command must not eat the next command's budget")
    ok, note, out = run_case(
        "slow", [("setuid", "find / -xdev -type f -perm -4000"),
                 ("kldstat", "kldstat -v")], 6)
    check(ok, f"interrogate() succeeded with a slow command ({note})")
    check((out["kldstat"] or "").strip().startswith("THE-REAL-OUTPUT-OF"),
          "the command AFTER the slow one still captured its output")

    print()
    if failures:
        print(f"{len(failures)} FAILED")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
