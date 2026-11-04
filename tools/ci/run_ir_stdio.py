#!/usr/bin/env python3
"""Boot THIS tree's vm.ufs.raw with autoboot (no loader typing) and run IR.

loader.conf on the image must set console=comconsole. Typing `boot` at the
loader overruns the 16550 and hangs module load. Extra virtio-net/9p PCI
at power-on also changes SeaBIOS/iPXE; those devices are hotplugged after
login from fsdev/netdev backends that are present with no PCI function.
"""
from __future__ import annotations

import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

TREE = Path("/home/odin/ParanoidBSD")
IMG = TREE / "out/vm.ufs.raw"
OUT = Path("/home/odin/pbsd-out")
VM = Path("/home/odin/vm")
LOG = VM / "ir-stdio.log"
TYPE_DELAY = 0.018

ANSI = re.compile(rb"\x1b\[[0-9;?]*[ -/]*[@-~]|\x1b[()][A-Za-z0-9]|\x1b[=>]")
LOGIN_PROMPT = re.compile(rb"login: *$")
PASSWORD_PROMPT = re.compile(rb"[Pp]assword:")
SHELL_PROMPT = re.compile(rb"(?:^|\n)[^\n]*[#$] $")
ORACLE_DONE = re.compile(rb"IR_ORACLE_DONE")
KERNEL = re.compile(rb"Copyright \(c\)|Timecounter|FreeBSD/amd64")


def _plain(b: bytes) -> bytes:
    return ANSI.sub(b"", b)


def _drain(proc, log, timeout: float = 0.3) -> bytes:
    chunk = proc.stdout.read(65536)
    if not chunk:
        time.sleep(timeout)
        return b""
    log.write(chunk)
    log.flush()
    sys.stdout.buffer.write(chunk)
    sys.stdout.buffer.flush()
    return chunk


def _send(proc, text: str, delay: float = TYPE_DELAY) -> bool:
    try:
        for byte in text.encode():
            proc.stdin.write(bytes([byte]))
            proc.stdin.flush()
            if delay:
                time.sleep(delay)
        return True
    except (BrokenPipeError, ValueError, OSError):
        return False


def _monitor(proc, log, line: str) -> None:
    _send(proc, "\x01c", delay=0.04)
    time.sleep(0.35)
    _drain(proc, log, 0.2)
    _send(proc, line + "\n", delay=0.012)
    time.sleep(0.8)
    _drain(proc, log, 0.3)
    _send(proc, "\x01c", delay=0.04)
    time.sleep(0.35)
    _drain(proc, log, 0.2)


def _expect(proc, log, patterns, deadline, buf: bytes = b"") -> tuple[int, bytes]:
    while time.time() < deadline:
        buf += _drain(proc, log)
        buf = buf[-262144:]
        vis = _plain(buf)
        for i, pat in enumerate(patterns):
            if pat.search(vis):
                return i, buf
        if proc.poll() is not None:
            return -1, buf
    return -1, buf


def main() -> int:
    signal.signal(signal.SIGHUP, signal.SIG_IGN)
    VM.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    if not IMG.is_file():
        print(f"FAIL no image at {IMG}", file=sys.stderr)
        return 2
    accel = ["-accel", "kvm"] if os.access("/dev/kvm", os.W_OK) else []
    cmd = [
        "qemu-system-x86_64",
        "-name", "pbsd-ir",
        "-machine", "q35",
        *accel,
        "-m", "4096",
        "-drive", f"file={IMG},format=raw,if=none,id=hd0",
        "-device", "virtio-blk-pci,drive=hd0",
        "-netdev", "user,id=net0,hostfwd=tcp:127.0.0.1:2222-:22",
        "-device", "virtio-net-pci,netdev=net0",
        "-fsdev", f"local,id=fs_pbsd,path={TREE},security_model=mapped-xattr",
        "-device", "virtio-9p-pci,fsdev=fs_pbsd,mount_tag=pbsd",
        "-fsdev", f"local,id=fs_out,path={OUT},security_model=mapped-xattr",
        "-device", "virtio-9p-pci,fsdev=fs_out,mount_tag=out",
        "-nographic",
        "-serial", "mon:stdio",
        "-pidfile", str(VM / "pbsd-ir.pid"),
    ]
    print("starting", " ".join(cmd), flush=True)
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        stdin=subprocess.PIPE,
    )
    assert proc.stdout is not None and proc.stdin is not None
    os.set_blocking(proc.stdout.fileno(), False)
    log = LOG.open("wb")
    keep = False
    try:
        print("== waiting for kernel then login (no loader typing)", flush=True)
        hit, buf = _expect(proc, log, [LOGIN_PROMPT, KERNEL], time.time() + 180)
        if hit == 1:
            print("== kernel on serial", flush=True)
            hit, buf = _expect(proc, log, [LOGIN_PROMPT], time.time() + 120, buf)
        if hit < 0:
            print("FAIL no login", file=sys.stderr)
            return 2
        time.sleep(0.3)
        _send(proc, "root\n")
        hit, buf = _expect(proc, log, [PASSWORD_PROMPT, SHELL_PROMPT], time.time() + 20)
        if hit == 0:
            _send(proc, "\n")
            hit, buf = _expect(proc, log, [SHELL_PROMPT], time.time() + 20)
        if hit < 0:
            print("FAIL no root shell", file=sys.stderr)
            return 2
        print("== guest commands", flush=True)
        cmds = [
            "echo LOGIN_OK",
            "sysctl hardening.insecure_kmod=1 || true",
            "sysctl hardening.pax.mprotect.status=0 || true",
            "kldload virtio_p9fs || kldload p9fs || true",
            "mkdir -p /mnt/pbsd /mnt/out",
            "mount -t p9fs pbsd /mnt/pbsd || mount -t virtio_p9fs pbsd /mnt/pbsd || true",
            "mount -t p9fs out /mnt/out || mount -t virtio_p9fs out /mnt/out || true",
            "ls /mnt/out/guest_ir_on_pbsd.sh /mnt/pbsd/tools/run_todo_passes.py",
            "echo MOUNTED",
            "sh /mnt/out/guest_ir_on_pbsd.sh",
        ]
        print("== guest commands", flush=True)
        for line in cmds[:-1]:
            _send(proc, line + "\n")
            _expect(proc, log, [SHELL_PROMPT], time.time() + 45)
        _send(proc, cmds[-1] + "\n")
        hit, buf = _expect(proc, log, [ORACLE_DONE], time.time() + 2400)
        if hit < 0:
            print("FAIL oracle did not finish", file=sys.stderr)
            return 2
        print("== IR_ORACLE_DONE", flush=True)
        _send(proc, "sync\n")
        time.sleep(1.0)
        _send(proc, "poweroff\n")
        deadline = time.time() + 25
        while proc.poll() is None and time.time() < deadline:
            _drain(proc, log, 0.4)
        keep = True
        return 0
    finally:
        if proc.poll() is None and keep:
            proc.terminate()
            try:
                proc.wait(timeout=8)
            except subprocess.TimeoutExpired:
                proc.kill()
        log.close()


if __name__ == "__main__":
    raise SystemExit(main())
