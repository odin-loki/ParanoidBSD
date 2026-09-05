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


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("--target", default="amd64")
    ap.add_argument("--timeout", type=int, default=300)
    ap.add_argument("--bios", default=None,
                    help="firmware image; arm64 and riscv need one")
    ap.add_argument("--log", default="boot.log")
    args = ap.parse_args()

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
                            stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL)
    assert proc.stdout is not None
    os.set_blocking(proc.stdout.fileno(), False)

    started = time.time()
    buf = b""
    progressed = False
    verdict = None
    with open(args.log, "wb") as log:
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

    proc.kill()
    proc.wait(timeout=10)
    elapsed = time.time() - started

    print(f"\n--- {elapsed:.0f}s, log in {args.log}")
    if verdict and verdict[0] == "OK":
        print(f"OK  booted: {verdict[1]}")
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
