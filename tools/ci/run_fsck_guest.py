#!/usr/bin/env python3
"""Boot the FreeBSD cloud image with the PBSD raw disk attached, fsck it."""
from __future__ import annotations

import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

CLOUD = "/home/odin/vm/FreeBSD-15.0-RELEASE-amd64-BASIC-CLOUDINIT-ufs.qcow2"
IMG = "/home/odin/ParanoidBSD/out/vm.ufs.raw"
LOG = Path("/home/odin/vm/fsck-guest.log")
TYPE_DELAY = 0.015
ANSI = re.compile(rb"\x1b\[[0-9;?]*[ -/]*[@-~]|\x1b[()][A-Za-z0-9]|\x1b[=>]")
LOGIN = re.compile(rb"login: *$")
PASSWORD = re.compile(rb"[Pp]assword:")
PROMPT = re.compile(rb"(?:^|\n)[^\n]*[#$] $")
DONE = re.compile(rb"FSCK_FINISHED_OK")


def _plain(b: bytes) -> bytes:
    return ANSI.sub(b"", b)


def _drain(proc, log, timeout=0.3) -> bytes:
    chunk = proc.stdout.read(65536)
    if not chunk:
        time.sleep(timeout)
        return b""
    log.write(chunk)
    log.flush()
    sys.stdout.buffer.write(chunk)
    sys.stdout.buffer.flush()
    return chunk


def _send(proc, text: str, delay: float = TYPE_DELAY) -> None:
    for byte in text.encode():
        proc.stdin.write(bytes([byte]))
        proc.stdin.flush()
        time.sleep(delay)


def _expect(proc, log, pats, deadline, buf=b""):
    while time.time() < deadline:
        buf += _drain(proc, log)
        buf = buf[-131072:]
        vis = _plain(buf)
        for i, p in enumerate(pats):
            if p.search(vis):
                return i, buf
        if proc.poll() is not None:
            return -1, buf
    return -1, buf


def main() -> int:
    signal.signal(signal.SIGHUP, signal.SIG_IGN)
    accel = ["-accel", "kvm"] if os.access("/dev/kvm", os.W_OK) else []
    cmd = [
        "qemu-system-x86_64", "-machine", "q35", *accel, "-m", "2048",
        "-drive", f"file={CLOUD},if=virtio",
        "-drive", f"file={IMG},format=raw,if=virtio",
        "-nographic", "-serial", "mon:stdio",
        "-pidfile", "/home/odin/vm/fsck-guest.pid",
    ]
    print("starting", " ".join(cmd), flush=True)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
    os.set_blocking(proc.stdout.fileno(), False)
    log = LOG.open("wb")
    try:
        hit, buf = _expect(proc, log, [LOGIN], time.time() + 240)
        if hit < 0:
            print("FAIL no login", file=sys.stderr)
            return 2
        _send(proc, "root\n")
        hit, buf = _expect(proc, log, [PASSWORD, PROMPT], time.time() + 20)
        if hit == 0:
            _send(proc, "pbsd\n")
            hit, buf = _expect(proc, log, [PROMPT], time.time() + 20)
            if hit < 0:
                _send(proc, "root\n")
                time.sleep(0.5)
                _send(proc, "\n")
                hit, buf = _expect(proc, log, [PROMPT], time.time() + 15)
        if hit < 0:
            print("FAIL no shell", file=sys.stderr)
            return 2
        for line in [
            "echo FSCK_BEGIN",
            "gpart show vtbd1",
            "fsck -y /dev/vtbd1p4",
            "echo FSCK_FINISHED_OK",
        ]:
            _send(proc, line + "\n", delay=0.02)
            if line.startswith("fsck"):
                hit, buf = _expect(proc, log, [PROMPT], time.time() + 180)
            elif "FINISHED" in line:
                hit, buf = _expect(proc, log, [DONE], time.time() + 30)
            else:
                hit, buf = _expect(proc, log, [PROMPT], time.time() + 30)
        print("fsck", "ok" if hit >= 0 else "FAIL", flush=True)
        _send(proc, "poweroff\n")
        time.sleep(5)
        return 0 if hit >= 0 else 2
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=8)
            except subprocess.TimeoutExpired:
                proc.kill()
        log.close()


if __name__ == "__main__":
    raise SystemExit(main())
