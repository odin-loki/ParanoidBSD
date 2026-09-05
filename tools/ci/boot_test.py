#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Boot a PBSD image under QEMU and check it actually comes up.

Every gate in this repository proves something compiles. None of them prove
the result runs. A kernel that links, a world that installs and an image that
mkimg accepts can still panic on the first page fault, and nothing here would
have noticed.

So: start the image, watch the serial console, and decide from what it says.

  * a marker from the list below means it booted;
  * a panic, a mountroot prompt or a trap means it did not, and the line is
    printed rather than summarised;
  * silence until the timeout means it did not get far enough to say anything,
    which is a different failure and is reported as one.

This runs on the Linux runner. The image is built on FreeBSD because release/
wants a FreeBSD host, but booting it is just QEMU and does not.

--run goes one step further and interrogates the system it just booted. A
booted kernel is the only place several of this repository's open questions
can be answered - what hardening.pax.mprotect.status actually defaults to,
what ships setuid in the built image rather than in the makefiles, whether
PAX is doing anything. Every one of those needs a shell, and a shell needs a
login.

It only applies to an image that has one. A memstick is the installer and
stops at bsdinstall's menu; release/vm.raw boots to a login prompt, which is
why the build script has a vm stage. With no --run the behaviour is exactly
what it was: watch, decide, exit.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import time

# Reaching any of these means the kernel started, mounted root, and ran init.
SUCCESS = [
    re.compile(rb"login:"),
    re.compile(rb"Welcome to (Paranoid|Hardened|Free)BSD"),
    re.compile(rb"Starting local daemons"),
]
# Seen early enough that it means the kernel is alive even if userland is not.
PROGRESS = [
    re.compile(rb"FreeBSD/[a-z0-9]+ \("),
    re.compile(rb"Booting\.\.\."),
    re.compile(rb"KDB: debugger backends"),
    re.compile(rb"real memory\s*="),
]
SHELL_PROMPT = re.compile(rb"(?:^|\n)[^\n]*[#$] $|(?:^|\n)# ")
LOGIN_PROMPT = re.compile(rb"login: *$")
PASSWORD_PROMPT = re.compile(rb"[Pp]assword: *$")

FAILURE = [
    (re.compile(rb"panic:.*"), "kernel panic"),
    (re.compile(rb"Fatal trap \d+"), "fatal trap"),
    (re.compile(rb"mountroot>"), "could not mount root"),
    (re.compile(rb"Manual root filesystem specification"), "could not mount root"),
]

QEMU = {
    "amd64": ["qemu-system-x86_64", "-machine", "q35", "-m", "2048"],
    "i386": ["qemu-system-i386", "-m", "1024"],
    "arm64": ["qemu-system-aarch64", "-machine", "virt", "-cpu", "cortex-a57",
              "-m", "2048"],
    "riscv": ["qemu-system-riscv64", "-machine", "virt", "-m", "2048"],
}


def _drain(proc, log, keep=65536, timeout=0.3):
    """Read whatever QEMU has said, mirror it, and return it."""
    chunk = proc.stdout.read(65536)
    if not chunk:
        time.sleep(timeout)
        return b""
    log.write(chunk)
    log.flush()
    sys.stdout.write(chunk.decode("utf-8", "replace"))
    sys.stdout.flush()
    return chunk


def _expect(proc, log, patterns, deadline, initial=b""):
    """Wait for any of `patterns`; return its index, or -1 on timeout.

    `initial` is what has already been printed. The prompt that ends the
    boot phase is the same prompt the login phase has to answer - the SUCCESS
    match is on `login:` - so starting with an empty buffer waits forever for
    something that is already on the screen. The first run of this code did
    exactly that.
    """
    buf = initial
    for i, pat in enumerate(patterns):
        if pat.search(buf):
            return i
    while time.time() < deadline:
        buf += _drain(proc, log)
        buf = buf[-65536:]
        for i, pat in enumerate(patterns):
            if pat.search(buf):
                return i
        if proc.poll() is not None:
            return -1
    return -1


def _send(proc, text):
    proc.stdin.write(text.encode())
    proc.stdin.flush()


def interrogate(proc, log, commands, user, password, timeout, outdir,
                initial=b""):
    """Log in and run each command, writing its output beside the log.

    Returns (ok, note). A failure here is reported apart from the boot
    verdict: the system booted either way, and "booted but could not log in"
    is a different fact from "did not boot".
    """
    deadline = time.time() + timeout
    print("\n  [logging in]\n")
    idx = _expect(proc, log, [LOGIN_PROMPT, SHELL_PROMPT], deadline,
                  initial=initial)
    if idx == -1:
        # Nothing on screen looked like a prompt; nudge it and look again.
        _send(proc, "\n")
        idx = _expect(proc, log, [LOGIN_PROMPT, SHELL_PROMPT], deadline)
    if idx == -1:
        return False, "no login prompt within the shell timeout"
    if idx == 0:
        _send(proc, user + "\n")
        idx = _expect(proc, log, [PASSWORD_PROMPT, SHELL_PROMPT], deadline)
        if idx == 0:
            _send(proc, password + "\n")
            if _expect(proc, log, [SHELL_PROMPT], deadline) == -1:
                return False, f"{user} did not get a shell after the password"
        elif idx == -1:
            return False, f"{user} did not get a shell"

    for n, (name, cmd) in enumerate(commands):
        # Two markers, not one, and the output is what lies between the LAST
        # BEGIN and the END after it.
        #
        # The obvious version counts occurrences of a single sentinel and
        # waits for the second - the shell echoes the command line, then
        # prints it. That works on a serial console, which is a tty, and
        # fails on anything that is not echoing, which is what a test harness
        # driving a pipe looks like. Keying on a count means the same code
        # behaves differently in the harness and in production, which is the
        # opposite of what a harness is for.
        begin = f"__PBSD_{n}_BEGIN__"
        end = f"__PBSD_{n}_END__"
        rx_begin = re.compile(begin.encode())
        rx_end = re.compile(end.encode())
        print(f"\n  [{name}] {cmd}\n")
        _send(proc, f"echo {begin}; {cmd} 2>&1; echo {end}\n")
        buf = b""
        done = False
        while time.time() < deadline:
            buf += _drain(proc, log)
            m_end = rx_end.search(buf)
            if m_end:
                starts = list(rx_begin.finditer(buf[:m_end.start()]))
                if starts:
                    body = buf[starts[-1].end():m_end.start()]
                    done = True
                    break
            if proc.poll() is not None:
                break
        if not done:
            return False, f"{name!r} did not finish within the shell timeout"
        out = os.path.join(outdir, f"{name}.txt")
        with open(out, "wb") as fh:
            fh.write(body.strip(b"\r\n") + b"\n")
        print(f"  [{name}] {len(body)} bytes -> {out}")

    return True, f"{len(commands)} command(s) run"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("--target", default="amd64")
    ap.add_argument("--timeout", type=int, default=300)
    ap.add_argument("--bios", default=None,
                    help="firmware image; arm64 and riscv need one")
    ap.add_argument("--log", default="boot.log")
    ap.add_argument("--user", default="root",
                    help="account to log in as when --run is given")
    ap.add_argument("--password", default="",
                    help="sent if a password prompt appears; empty by default")
    ap.add_argument("--run", action="append", default=[], metavar="NAME=CMD",
                    help="after login, run CMD and write its output to "
                         "NAME.txt beside the log. Repeatable.")
    ap.add_argument("--shell-timeout", type=int, default=120,
                    help="seconds to allow for login and all --run commands")
    args = ap.parse_args()

    commands = []
    for spec in args.run:
        name, sep, cmd = spec.partition("=")
        if not sep or not name.strip() or not cmd.strip():
            print(f"FAIL --run needs NAME=CMD, got {spec!r}", file=sys.stderr)
            return 2
        commands.append((name.strip(), cmd.strip()))

    if not os.path.isfile(args.image):
        print(f"FAIL no image at {args.image}", file=sys.stderr)
        return 2
    if args.target not in QEMU:
        print(f"FAIL no QEMU recipe for {args.target}", file=sys.stderr)
        return 2
    if shutil.which(QEMU[args.target][0]) is None:
        print(f"FAIL {QEMU[args.target][0]} not installed", file=sys.stderr)
        return 2

    cmd = list(QEMU[args.target])
    if args.bios:
        cmd += ["-bios", args.bios]
    cmd += [
        "-drive", f"file={args.image},format=raw,if=none,id=hd0",
        "-device", "virtio-blk-pci,drive=hd0",
        "-nographic", "-serial", "mon:stdio",
        "-no-reboot",
    ]
    size = os.path.getsize(args.image)
    print(f"booting {args.image} ({size / 1e6:.0f} MB) as {args.target}")
    print("  " + " ".join(cmd) + "\n")

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            stdin=subprocess.PIPE if commands
                            else subprocess.DEVNULL)
    assert proc.stdout is not None
    os.set_blocking(proc.stdout.fileno(), False)

    started = time.time()
    buf = b""
    progressed = False
    verdict = None
    # Not a with-block: the interrogation phase below writes to the same
    # log, and closing it at the end of the boot loop made the first run of
    # this code die with "write to closed file".
    log = open(args.log, "wb")
    try:
        while time.time() - started < args.timeout:
            chunk = proc.stdout.read(4096)
            if not chunk:
                if proc.poll() is not None:
                    break
                time.sleep(0.2)
                continue
            log.write(chunk)
            log.flush()
            sys.stdout.write(chunk.decode("utf-8", "replace"))
            sys.stdout.flush()
            buf += chunk
            buf = buf[-65536:]

            for pat, why in FAILURE:
                m = pat.search(buf)
                if m:
                    verdict = ("FAIL", why, m.group(0).decode("utf-8", "replace"))
                    break
            if verdict:
                break
            if not progressed and any(p.search(buf) for p in PROGRESS):
                progressed = True
                print("\n  [kernel is alive]\n")
            if any(p.search(buf) for p in SUCCESS):
                verdict = ("OK", "reached userland", "")
                break

        shell = None
        if commands and verdict and verdict[0] == "OK":
            try:
                shell = interrogate(proc, log, commands, args.user,
                                    args.password, args.shell_timeout,
                                    os.path.dirname(
                                        os.path.abspath(args.log)),
                                    initial=buf)
            except (BrokenPipeError, OSError) as e:
                shell = (False, f"the console went away: {e}")
    finally:
        log.close()

    proc.kill()
    proc.wait(timeout=10)
    elapsed = time.time() - started

    print(f"\n--- {elapsed:.0f}s, log in {args.log}")
    if verdict and verdict[0] == "OK":
        print(f"OK  booted: {verdict[1]}")
        if shell is not None:
            ok, note = shell
            if ok:
                print(f"OK  shell: {note}")
            else:
                print(f"FAIL booted, but the shell did not work out: {note}")
                print("     That is a different fact from a boot failure and")
                print("     is reported as one.")
                return 1
        return 0
    if verdict:
        print(f"FAIL {verdict[1]}")
        print(f"     {verdict[2]}")
        return 1
    if progressed:
        print("FAIL the kernel started but never reached userland "
              f"within {args.timeout}s.")
        print("     That is a hang after boot, not a boot failure - the tail "
              "of the log is where it stopped.")
        return 1
    print(f"FAIL nothing on the serial console in {args.timeout}s.")
    print("     The loader did not run, or the image is not bootable at all.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
