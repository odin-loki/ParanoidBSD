#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Boot a PBSD image under QEMU and check it actually comes up.

Every gate in this repository proves something compiles. None of them prove
the result runs. A kernel that links, a world that installs and an image that
mkimg accepts can still panic on the first page fault, and nothing here would
have noticed.

So: start the image, watch the serial console, and decide from what it says.

  * three phases are tracked apart - the loader ran, the kernel started,
    userland is talking - and only the third is a boot;
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
stops at bsdinstall's menu; release/vm.ufs.raw boots to a login prompt, which
is why the build script has a vm stage. With no --run the behaviour is exactly
what it was: watch, decide, exit.

One more thing has to happen before any of that is possible, and it took a
run to notice. The x86 loader writes to the serial port, so its menu arrives
here - but the menu itself says

    5. Cons: Video

and `console` is what the loader hands the kernel in kenv. So the loader
talks to us and the kernel it starts does not: everything from
"Copyright (c) 1992" onwards goes to a VGA device that `-nographic` is not
showing anyone. A run left alone would sit through its whole timeout, see
nothing after the menu, and report "the loader ran and the kernel never
started" - which would be a lie of exactly the kind this file already has
one comment about.

So the boot loop drives the menu: 3 to escape to the loader prompt, then
`set console="comconsole,vidconsole"` and `boot`. --loader-console changes
the value and an empty one turns the whole thing off.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import time

# Three phases, kept apart on purpose.
#
# The first version of this had one SUCCESS list and it contained
#
#     re.compile(rb"Welcome to (Paranoid|Hardened|Free)BSD")
#
# which is the title of the BOOT LOADER MENU. Run 15 matched it four seconds
# in, reported "OK booted: reached userland", and exited - and the log
# contains no "Copyright (c) 1992", no "real memory  =" and no trademark
# line, because the kernel had not been loaded yet. The tool declared the
# system booted while it was sitting in /boot/lua drawing a menu.
#
# That is the most consequential possible version of the mistake this
# project keeps making: a pattern that matches something NEAR the answer.
# So the phases are separate, the verdict is the furthest one reached, and
# success requires the third.

# The loader ran. Says nothing about the kernel.
LOADER = [
    re.compile(rb"FreeBSD/[a-z0-9]+ bootstrap loader"),
    re.compile(rb"Loading /boot/(?:defaults/)?loader\.conf"),
    re.compile(rb"Welcome to (?:Paranoid|Hardened|Free)BSD"),  # the menu title
    re.compile(rb"Consoles: internal video/keyboard"),
]

# The kernel started. Says nothing about userland.
KERNEL = [
    re.compile(rb"Copyright \(c\) 199[0-9]-"),
    re.compile(rb"FreeBSD is a registered trademark"),
    re.compile(rb"real memory\s*="),
    re.compile(rb"avail memory\s*="),
    re.compile(rb"KDB: debugger backends"),
    re.compile(rb"Timecounter \""),
]

# Typed at the console when the kernel has gone quiet, to tell "nothing is
# running" apart from "something is running and saying nothing".
#
# start_init() printed `start_init: trying /sbin/init` and then produced
# nothing for 420 seconds. That is consistent with init hanging AND with
# init running fine while printing nothing - and a shell exec'd as PID 1
# via init_path prints no banner at all, so the second case would have been
# recorded as a failure. A marker echoed back is proof a shell is there;
# silence after it is evidence there is not.
ALIVE = "__PBSD_ALIVE__"

# init ran and something in userland is talking. This is a boot.
USERLAND = [
    re.compile(ALIVE.encode() + rb"\s*$", re.M),
    re.compile(rb"login:"),
    re.compile(rb"Starting local daemons"),
    re.compile(rb"Enter full pathname of shell"),
    # bsdinstall, which is where an installer image ends up
    re.compile(rb"Welcome to the (?:Paranoid|Hardened|Free)BSD installer"),
    re.compile(rb"\bInstall\b.{0,40}\bShell\b.{0,40}\bLive"),
    re.compile(rb"bsdinstall"),
]

PHASES = [("loader", LOADER), ("kernel", KERNEL), ("userland", USERLAND)]

# Disk providers the kernel announces as it attaches them. If root cannot be
# mounted, WHICH of these appeared is the whole diagnosis.
DISKS = re.compile(rb"GEOM: new disk (\w+)|(\bvtbd\d+|\bada\d+|\bda\d+|"
                   rb"\bcd\d+|\bmd\d+)\b")


def _explain_mountroot(vis: bytes) -> None:
    """Say WHY root could not be mounted, not just that it could not.

    Run 40 reached `Manual root filesystem specification:` and the boot test
    reported "could not mount root" correctly and in nine seconds. What it
    could not say was that the kernel had attached a CD-ROM and nothing
    else - because HARDENEDBSD-MINIMAL, the workflow's DEFAULT kernconf,
    includes MINIMAL and HARDENEDBSD-CORE and therefore no virtio at all,
    while HARDENEDBSD includes GENERIC and gets `device virtio_blk`.

    QEMU was handing it a virtio-blk disk the kernel had no driver for. That
    is a kernel/VM mismatch and not a PBSD fault, and half an hour went into
    reading a boot log to find it out. So the probe says it.
    """
    seen = set()
    for m in DISKS.finditer(vis):
        seen.add((m.group(1) or m.group(2)).decode())
    print("\n  why root could not be mounted")
    print(f"    disk devices the kernel attached: "
          f"{', '.join(sorted(seen)) if seen else 'NONE'}")
    if not any(d.startswith("vtbd") for d in seen):
        print("    no vtbd* - this kernel has no virtio_blk, and the boot")
        print("    image is presented to QEMU as -device virtio-blk-pci.")
        print("    HARDENEDBSD-MINIMAL includes MINIMAL + HARDENEDBSD-CORE")
        print("    and has no virtio; HARDENEDBSD includes GENERIC and does.")
        print("    Re-run with kernconf=HARDENEDBSD.")

FAILURE = [
    (re.compile(rb"panic:.*"), "kernel panic"),
    (re.compile(rb"Fatal trap \d+"), "fatal trap"),
    (re.compile(rb"mountroot>"), "could not mount root"),
    (re.compile(rb"Manual root filesystem specification"), "could not mount root"),
]

# The menu is drawn with cursor positioning and colour, and the highlight
# for each hotkey is INSIDE the word:
#
#     3. \x1b[0;1mEsc\x1b[0mape to loader prompt
#
# so a pattern for "Escape to loader prompt" matches nothing, and would have
# matched nothing quietly - the boot would simply have gone on to Video and
# the timeout would have blamed the kernel. Strip the escape sequences and
# match what a person reading the screen would read.
ANSI = re.compile(rb"\x1b\[[0-9;?]*[ -/]*[@-~]|\x1b[()][A-Za-z0-9]|\x1b[=>]")


def _plain(b: bytes) -> bytes:
    """The buffer with terminal control sequences removed."""
    return ANSI.sub(b"", b)


# The boot loader menu, and the loader prompt behind option 3. Matching the
# menu on "Escape to loader prompt" rather than on its title is deliberate:
# the title is the string that produced this file's false positive, and it
# is also drawn by the beastie logo, the version banner and nothing else
# that can be typed at. The line naming the key is the line that means the
# menu is accepting keys.
LOADER_MENU = re.compile(rb"Escape to loader prompt")
LOADER_READY = re.compile(rb"Type '\?' for a list of commands")

SHELL_PROMPT = re.compile(rb"(?:^|\n)[^\n]*[#$] $|(?:^|\n)# ")
LOGIN_PROMPT = re.compile(rb"login: *$")
PASSWORD_PROMPT = re.compile(rb"[Pp]assword: *$")

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


# Seconds between characters when typing at the guest.
#
# The first version wrote the whole command in one write() and the loader
# received exactly this:
#
#     set console="co
#
# Fifteen characters of a forty-character line, and then seven minutes of
# nothing. The 16550 the loader reads has a sixteen-byte FIFO and no flow
# control, and the loader polls it between other work, so a burst overruns
# it and the rest of the line is gone. Nothing reports the overrun: the
# command simply never completes, `boot` is never reached, and the run times
# out looking like a kernel that did not start.
#
# So type, do not paste.
TYPE_DELAY = 0.015


def _send(proc, text, delay: float = TYPE_DELAY) -> bool:
    """Type at the guest, one byte at a time. False if the console is gone."""
    try:
        for byte in text.encode():
            proc.stdin.write(bytes([byte]))
            proc.stdin.flush()
            if delay:
                time.sleep(delay)
        return True
    except (BrokenPipeError, ValueError, OSError):
        return False


def _steer_loader(proc, log, console: str, timeout: float,
                  extra: list[str] | None = None):
    """At the loader prompt: set the console, confirm it, and boot.

    Returns (confirmed, note). `confirmed` is True only when the loader
    echoed the whole command back - which is the only evidence available
    that it received the whole command. The verdict text depends on this:
    claiming the console was set when the line was truncated is how the
    memstick run came to report "it is the kernel not printing" about a
    kernel that had never been asked to start.
    """
    # Anything else the caller wants set goes first, so the console command
    # is the last thing sent and its receipt is unambiguous.
    #
    # Each one is confirmed the same way the console command is. Run 22 set
    # boot_verbose and init_path; boot_verbose plainly took effect (the
    # kernel printed `start_init: trying`, which only bootverbose does) and
    # init_path plainly did not (it tried /sbin/init). Which of "never
    # typed", "typed and dropped" or "arrived and ignored" that was could
    # not be read out of the log, because a verbose boot is hundreds of
    # lines and the steering is above all of them. So the tool records it.
    for spec in extra or []:
        name, _, value = spec.partition("=")
        line = f'set {name.strip()}="{value.strip()}"'
        print(f"  [loader: {line}]")
        if not _send(proc, line + "\n"):
            LOADER_SENT.append((line, False, "console went away"))
            return False, "the console went away while typing"
        ok, why = _await_receipt(proc, log, line, min(timeout, 10.0))
        LOADER_SENT.append((line, ok, why))

    cmd = f'set console="{console}"'
    if not _send(proc, cmd + "\n"):
        return False, "the console went away while typing"

    ok, why = _await_receipt(proc, log, cmd, timeout)
    LOADER_SENT.append((cmd, ok, why))
    if ok:
        _send(proc, "boot\n")
        return True, f"console set, {why}"
    # Boot anyway. A run that watches an unsteered boot is still worth more
    # than one that sits at the prompt, and the verdict will say the console
    # was never confirmed rather than claiming it was set.
    _send(proc, "boot\n")
    return False, why


# What was typed at the loader prompt, and whether the loader said so.
LOADER_SENT: list[tuple[str, bool, str]] = []


def _await_receipt(proc, log, cmd: str, timeout: float):
    """Wait for the loader to acknowledge `cmd`. Returns (ok, why).

    # Two independent receipts, because neither is guaranteed on its own:
    #
    #   * the echo. The loader echoes what it receives, so seeing the tail of
    #     the command come back means the whole line arrived. This is the
    #     signal the real x86 loader gives, and the one whose absence -
    #     `set console="co` and nothing more - is the bug this exists for.
    #   * a fresh `OK ` prompt. The loader prints one after each command it
    #     accepts, and prints none for a line it never saw terminated. A
    #     console configured not to echo still gives this one.
    #
    # The buffer starts empty, so an `OK ` found here is a new one and not
    # the prompt that got us here.
    """
    tail = re.compile(re.escape(cmd[-16:]).encode())
    prompt = re.compile(rb"OK\s*$")
    deadline = time.time() + timeout
    buf = b""
    while time.time() < deadline:
        buf += _drain(proc, log)
        vis = _plain(buf)
        if tail.search(vis):
            return True, "confirmed by echo"
        if prompt.search(vis):
            return True, "confirmed by a fresh loader prompt"
        if proc.poll() is not None:
            return False, "qemu exited at the loader prompt"
    return False, f"the loader never echoed the whole command in {timeout:.0f}s"


# What the kernel debugger said, printed at the END for the same reason the
# loader record is: a verbose boot is hundreds of lines and no tail reaches
# above them.
DDB_REPORT: list[tuple[str, str]] = []

# DDB's prompt, and its pager. db_command.c:552 prints "db> "; db_output.c:257
# prints "--More--\r" and waits for a key.
DDB_MORE = re.compile(rb"--More--")


def _ddb_read(proc, log, deadline):
    """Read until the db> prompt, answering the pager. -> (bytes, reached)."""
    buf = b""
    while time.time() < deadline:
        buf += _drain(proc, log)
        buf = buf[-262144:]
        flat = _plain(buf)
        if DDB_MORE.search(flat[-256:]):
            # Space is another page. Drop the marker so it does not match
            # again on the next pass through the same tail.
            _send(proc, " ", delay=0)
            buf = DDB_MORE.sub(b"", buf)
            continue
        if flat.rstrip().endswith(b"db>"):
            return buf, True
        if proc.poll() is not None:
            return buf, False
        time.sleep(0.1)
    return buf, False


def break_to_ddb(proc, log, timeout: float, cmds, rounds: int = 1,
                 gap: float = 10.0) -> bool:
    """Type ALT_BREAK_TO_DEBUGGER and run commands at the db> prompt.

    This is the only way left to tell the two remaining readings of "one
    `start_init: trying` line and then silence" apart. kern_execve() either
    never returned, or it returned EJUSTRETURN and init is running and
    cannot write - and no boot log can separate those, because both are
    silence.

    The sequence is three bytes, from sys/kern/subr_kdb.c:327: CR, then '~',
    then ^B. sys/dev/uart/uart_core.c:353 hands every received byte to
    kdb_alt_break(), so the console this function is already typing into is
    the one that carries it. amd64's HARDENEDBSD includes sys/conf/std.debug
    (options DDB, options ALT_BREAK_TO_DEBUGGER) and GENERIC has options KDB.

    `ps` then says whether a process 1 exists and what it waits on, and `bt`
    says where the thread is. Either answer ends the question - run 31 got
    `pid 1 ... RL CPU 0 [init]` and an `--- interrupt, rip = 0x2c2994c9fd0`
    frame, a userland address, so kern_execve() had returned EJUSTRETURN and
    init was on the CPU.

    `rounds` > 1 continues after each pass and breaks again, because one
    break is one sample: a tight spin at a single instruction, a loop, and
    slow progress are indistinguishable from one backtrace and obvious from
    two.
    """
    for r in range(1, max(1, rounds) + 1):
        tag = "" if rounds == 1 else f" [{r}/{rounds}]"
        _drain(proc, log)
        if not _send(proc, "\r~\x02"):
            DDB_REPORT.append((f"break sequence{tag}",
                               "the console went away"))
            return False
        _, reached = _ddb_read(proc, log, time.time() + timeout)
        if not reached:
            DDB_REPORT.append((
                f"break sequence{tag}",
                f"no db> prompt within {timeout:.0f}s. Either the kernel is "
                "not reading the console, or ALT_BREAK_TO_DEBUGGER is not in "
                "this kernel, or it is too wedged to take a trap."))
            return False
        DDB_REPORT.append((f"break sequence{tag}", "db> reached"))

        for cmd in cmds:
            if not _send(proc, cmd + "\n"):
                DDB_REPORT.append((cmd + tag, "the console went away"))
                return False
            out, ok = _ddb_read(proc, log, time.time() + timeout)
            text = _plain(out).decode("utf-8", "replace")
            # Drop the echo of the command itself and the trailing prompt.
            text = text.replace("\r", "")
            DDB_REPORT.append((cmd + tag, text.strip() or "(no output)"))
            if not ok:
                DDB_REPORT.append((cmd + tag,
                                   f"...and no db> prompt after "
                                   f"{timeout:.0f}s"))
                return False

        if r < rounds:
            # Continue, let it run, and break again. One break is one
            # sample of rip; two say whether it moved - a tight spin at one
            # instruction, a loop, or slow progress all look identical from
            # a single backtrace.
            if not _send(proc, "c\n"):
                DDB_REPORT.append((f"continue{tag}",
                                   "the console went away"))
                return False
            time.sleep(gap)
    return True


# `--- interrupt, rip = 0x3d6201b9fd0, rsp = 0x6ab2caaa2f60, rbp = ... ---`
# is DDB saying the trap came from somewhere other than kernel code. The
# addresses are what say WHERE, and reading them is the whole point of
# breaking in.
IFRAME = re.compile(
    r"---\s*interrupt,\s*rip\s*=\s*0x([0-9a-f]+)"
    r"(?:,\s*rsp\s*=\s*0x([0-9a-f]+))?"
    r"(?:,\s*rbp\s*=\s*0x([0-9a-f]+))?", re.I)

# `  map entry 0xfff...: start=0x6054539000, end=0x605453c000, eflags=0xc0c,`
# `   prot=1/1/copy, object=0xfff..., offset=0x0, copy (needed)`
#
# eflags is printed WITHOUT the 0x when it is zero - `eflags=0,` - and four
# of run 43's twelve entries are like that. Requiring 0x silently dropped
# them, which is the failure mode this whole function exists to avoid.
MAPENT = re.compile(
    r"map entry\s+0x[0-9a-f]+:\s*start=0x([0-9a-f]+),\s*end=0x([0-9a-f]+),"
    r"\s*eflags=(?:0x)?([0-9a-f]+)[^\n]*\n\s*prot=(\d)/(\d)/(\w+)"
    r"[^\n]*?offset=0x([0-9a-f]+)", re.I)

# Kernel text on amd64 lives at the top of the address space. Anything
# below the canonical hole is a user address.
KERNBASE = 0xffff800000000000


def _load_symbols(path):
    """`nm -n` output: `0000000000005f80 T reloc_nonplt_self`, sorted.

    Anything that is not an address and a name is skipped, so a link map or
    an `nm` with extra columns still works.
    """
    out = []
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                parts = line.split()
                if len(parts) < 3:
                    continue
                try:
                    addr = int(parts[0], 16)
                except ValueError:
                    continue
                out.append((addr, parts[-1]))
    except OSError as e:
        print(f"  [could not read symbols from {path}: {e}]")
        return None
    out.sort()
    return out or None


def _nearest_symbol(symbols, va):
    """The last symbol at or below va, its offset, and how far it extends.

    The span matters. Run 56 printed `nearest preceding symbol: _rtld+0x10`
    and that was taken at face value for some time -- but the next symbol
    was 32 bytes later, and _rtld() is 534 lines of C. A 32-byte _rtld is
    not _rtld, so the name was wrong and the confident sentence carrying
    it was worse than no name at all.

    "Nearest preceding" is all a symbol table can give; whether that is an
    ANSWER depends on how big the symbol is, so say how big it is.
    """
    lo, hi = 0, len(symbols)
    while lo < hi:
        mid = (lo + hi) // 2
        if symbols[mid][0] <= va:
            lo = mid + 1
        else:
            hi = mid
    if lo == 0:
        return None, 0, None
    addr, name = symbols[lo - 1]
    span = symbols[lo][0] - addr if lo < len(symbols) else None
    return name, va - addr, span


def _load_disasm(path):
    """`objdump -d` output, as {elf virtual address: text of the line}.

    A symbol name is a guess about which function an address is in. The
    instruction AT that address is not a guess, and when the process is
    wedged at one rip forever the instruction is the whole question --
    a load, a store, an indirect call and an ud2 fail in four different
    ways and only one of them is a fault loop.
    """
    out = {}
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                # `  5fd0:\t48 8b 04 25 ...\tmovq  0x0, %rax`
                head, sep, _ = line.partition(":")
                if not sep:
                    continue
                head = head.strip()
                if not head or any(c not in "0123456789abcdefABCDEF"
                                   for c in head):
                    continue
                out[int(head, 16)] = line.rstrip("\n")
    except OSError as e:
        print(f"  [could not read disassembly from {path}: {e}]")
        return None
    return out or None


def _print_disasm(disasm, va, window=6):
    """The instruction at va, with a few either side for context."""
    addrs = sorted(disasm)
    if va not in disasm:
        print(f"      0x{va:x} is not an instruction boundary in the "
              "supplied disassembly")
        print("      (wrong object, or the rip is mid-instruction).")
        return
    i = addrs.index(va)
    print("      instructions around the rip:")
    for a in addrs[max(0, i - window):i + window + 1]:
        print(f"      {'>>' if a == va else '  '} {disasm[a]}")


def _explain_ddb(report, symbols=None, disasm=None) -> None:
    """Say what the debugger output MEANS, not just that it happened.

    Run 43 printed all of this and then concluded "the kernel started and
    never reached userland" - which the same output disproves. The frame
    said

        --- interrupt, rip = 0x3d6201b9fd0, rsp = 0x6ab2caaa2f60 ---

    a USER address, so pid 1 was on the CPU in userland; and `show procvm 1`
    put that rip inside the second vnode-backed r-x mapping, 0xfd0 bytes in,
    with no third one - the run-time linker executing, before it had mapped
    a single shared library. That is a completely different failure from a
    kernel hang and the verdict named the wrong one.

    This is the same shape as run 40, where the correct answer was on
    screen and the wrong thing was read, and it cost six runs. So it is
    read here instead.
    """
    frames, maps = [], []
    for cmd, out in report:
        # ONLY pid 1's backtrace. Run 56 collected every `--- interrupt,
        # rip = ...` line in the report, including the one from the bare
        # `bt` of pid 12 (an ithread, in the kernel), and then counted it
        # as a sample of pid 1. Three identical userland frames plus one
        # unrelated kernel frame came out as "4 samples at 2 distinct
        # rip(s): it is running, not stuck at a single instruction" - the
        # exact opposite of what the output said. Every frame here has to
        # come from a trace that named pid 1.
        if "Tracing pid 1 " in out:
            for m in IFRAME.finditer(out):
                frames.append((int(m.group(1), 16),
                               int(m.group(2), 16) if m.group(2) else None))
        if "procvm" in cmd:
            ents = []
            for m in MAPENT.finditer(out):
                ents.append({"start": int(m.group(1), 16),
                             "end": int(m.group(2), 16),
                             "eflags": int(m.group(3), 16),
                             "prot": int(m.group(4)),
                             "share": m.group(6).lower(),
                             "offset": int(m.group(7), 16)})
            if ents:
                maps.append(ents)
    if not frames:
        return

    print("    what the debugger output says:")

    rip, rsp = frames[0]
    if rip >= KERNBASE:
        print(f"      rip=0x{rip:x} is a KERNEL address: the thread was in "
              "the kernel.")
    else:
        print(f"      rip=0x{rip:x} is a USER address. pid 1 was executing "
              "in USERLAND -")
        print("      kern_execve() returned and the image is running. This "
              "is NOT a")
        print("      kernel hang and not a failure to reach userland.")

    # Two samples of the same rip and the same rsp is a spin, not progress.
    uniq_rip = {f[0] for f in frames}
    uniq_rsp = {f[1] for f in frames if f[1] is not None}
    if len(frames) > 1:
        if len(uniq_rip) == 1 and len(uniq_rsp) <= 1:
            print(f"      {len(frames)} samples, all at the SAME rip and the "
                  "same rsp: it is")
            print("      spinning at one instruction, not making slow "
                  "progress. A fault")
            print("      that re-faults on delivery looks exactly like this.")
        else:
            print(f"      {len(frames)} samples at {len(uniq_rip)} distinct "
                  "rip(s): it is running,")
            print("      not stuck at a single instruction.")

    if rip >= KERNBASE or not maps:
        return

    ents = maps[0]
    # Executable PRIVATE mappings, in address order: the loaded objects.
    # One is the executable, one is the rtld, and each shared library adds
    # another. The `share` ones are excluded deliberately - the kernel's
    # shared page carries the signal trampoline at prot=5/5/share and is
    # executable without being anything the rtld loaded.
    text = [e for e in ents if e["prot"] & 4 and e["share"] != "share"]
    hit = next((e for e in ents if e["start"] <= rip < e["end"]), None)
    if hit is not None:
        idx = text.index(hit) + 1 if hit in text else None
        where = (f"executable mapping #{idx} of {len(text)}"
                 if idx else "a NON-EXECUTABLE mapping (!)")
        print(f"      rip is 0x{rip - hit['start']:x} bytes into {where} "
              f"(0x{hit['start']:x}-0x{hit['end']:x}).")
        if idx == 2 and len(text) == 2:
            print("      Two executable mappings means the binary and the "
                  "run-time linker")
            print("      and nothing else: ld-elf.so.1 has not mapped a "
                  "single shared")
            print("      library yet. The rtld is where this is stuck, "
                  "which is also why")
            print("      a STATIC init (/rescue/init, NO_SHARED=yes) gets "
                  "past it.")
        # ASLR put this object somewhere random, so the raw rip means
        # nothing on its own. The object's LOAD BASE is the start of its
        # lowest mapping, which is the one whose file offset is 0; rip minus
        # that is the ELF virtual address, which IS meaningful - it can be
        # looked up in the link map the build already writes
        # (libexec/rtld-elf/Makefile:79 asks the linker for
        # ld-elf.so.1.map).
        base = None
        for e in ents:
            if e["start"] <= hit["start"] and e["offset"] == 0:
                if base is None or e["start"] > base:
                    base = e["start"]
        if base is not None:
            va = rip - base
            print(f"      load base 0x{base:x}, so its ELF virtual address "
                  f"is 0x{va:x}.")
            print("      Look that up in the object's link map or `nm -n` "
                  "output to name")
            print("      the function; ASLR makes the raw rip meaningless "
                  "and this is not.")
            if symbols:
                name, off, span = _nearest_symbol(symbols, va)
                if name:
                    print(f"      nearest preceding symbol: {name}"
                          + (f"+0x{off:x}" if off else "")
                          + (f"  (that symbol spans {span} bytes)"
                             if span is not None else ""))
                    # A name is only an answer if the symbol is big enough
                    # to be the function you think it is. Run 56 reported
                    # `_rtld+0x10` from a 32-byte symbol, and _rtld() is
                    # 534 lines of C - so the table's idea of _rtld was not
                    # the function, and the name sent the reading the wrong
                    # way for as long as nobody checked the next entry.
                    if span is not None and span <= 64:
                        print("      NOTE: that is a very small symbol. The "
                              "next entry follows")
                        print(f"      {span} bytes later, so this name is "
                              "'the closest label', not")
                        print("      necessarily the function the rip is "
                              "in. Trust --disasm over it.")
                else:
                    print("      no symbol at or below that address in the "
                          "supplied table.")
            if disasm:
                _print_disasm(disasm, va)
    else:
        print(f"      rip=0x{rip:x} is in NO mapping of pid 1 - it jumped "
              "somewhere unmapped.")
    print(f"      pid 1 has {len(ents)} map entries, {len(text)} of them "
          "executable.")


def interrogate(proc, log, commands, user, password, timeout, outdir,
                initial=b""):
    """Log in and run each command, writing its output beside the log.

    Returns (ok, note). A failure here is reported apart from the boot
    verdict: the system booted either way, and "booted but could not log in"
    is a different fact from "did not boot".

    `timeout' bounds the login AND is the per-command budget, rather than a
    single deadline shared by everything. Run 59 shared it: `find / -xdev'
    over a 7.5GB image used the lot, and the two commands after it were
    typed into the still-running find's stdin, which is what the log shows.
    A slow command is not a hung one, and it should not spend the next
    command's time.
    """
    deadline = time.time() + timeout
    per_cmd = timeout
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
        #
        # AND THE MARKERS MUST MATCH AT THE START OF A LINE.
        #
        # Two markers do not by themselves solve the echo problem, because
        # the echoed command line contains BOTH of them:
        #
        #   root@freebsd:~ # echo __PBSD_0_BEGIN__; uname -a 2>&1; echo __PBSD_0_END__
        #
        # so the very first read matched END inside the echo, took the text
        # between the two as the body, and wrote the COMMAND to uname.txt
        # instead of its output. Run 59's uname.txt is
        #
        #   ; uname -a 2>&1; echo
        #
        # in full. hardening.txt escaped only by accident: its echoed line
        # was long enough to wrap at eighty columns, arriving as
        # `echo __PBSD_1_END __' with a space in it, which did not match -
        # so that one command waited for the real output and captured 30
        # lines of sysctl. One of five right, for a reason that had nothing
        # to do with the code being right.
        #
        # The output markers begin a line; the echo never does, since the
        # prompt and `echo ' precede it. Anchoring is the whole fix.
        begin = f"__PBSD_{n}_BEGIN__"
        end = f"__PBSD_{n}_END__"
        rx_begin = re.compile(rb"(?m)^\r?" + begin.encode())
        rx_end = re.compile(rb"(?m)^\r?" + end.encode())
        print(f"\n  [{name}] {cmd}\n")
        _send(proc, f"echo {begin}; {cmd} 2>&1; echo {end}\n")
        buf = b""
        done = False
        # Each command gets its own budget. `find / -xdev' over a 7.5GB
        # image is not a hung shell, and run 59 spent the whole shared
        # deadline on it, then reported the NEXT command as the failure.
        deadline = time.time() + per_cmd
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
    ap.add_argument("--loader-console", default="comconsole,vidconsole",
                    help="at the loader menu, escape to the loader prompt "
                         "and set this as the console before booting. The "
                         "menu's own default is Video, which sends every "
                         "line the kernel prints to a VGA device nobody is "
                         "looking at. Empty string to leave the menu alone.")
    ap.add_argument("--loader-set", action="append", default=[],
                    metavar="NAME=VALUE",
                    help="an extra `set NAME=\"VALUE\"` at the loader prompt, "
                         "before boot. Repeatable. boot_verbose=YES makes "
                         "start_init() name each init it tries; "
                         "boot_single=YES makes init exec a shell instead of "
                         "running rc, which separates a broken init from a "
                         "broken rc.")
    ap.add_argument("--ddb-on-hang", action="store_true",
                    help="if the kernel is up and the console pokes get "
                         "nothing back, type the ALT_BREAK_TO_DEBUGGER "
                         "sequence (CR ~ ^B, subr_kdb.c:327) and run "
                         "--ddb-cmd at the db> prompt. This is what tells a "
                         "kern_execve() that never returned from an init(8) "
                         "that runs and cannot write: no boot log can, "
                         "because both are silence.")
    ap.add_argument("--symbols",
                    help="`nm -n` output for the object the hang is in "
                         "(ld-elf.so.1, say). A user rip is meaningless "
                         "under ASLR; with this, its ELF virtual address "
                         "is resolved to the nearest preceding symbol.")
    ap.add_argument("--disasm",
                    help="`objdump -d` output for the same object as "
                         "--symbols. A symbol name is a guess about which "
                         "function an address is in; the instruction at "
                         "that address is not, and for a process wedged at "
                         "one rip it is the whole question.")
    ap.add_argument("--ddb-cmd", action="append", default=[],
                    metavar="COMMAND",
                    help="a command to run at db>. Repeatable. Default: "
                         "`ps` (is there a process 1, and what does it wait "
                         "on) then `bt` (where is the thread).")
    ap.add_argument("--ddb-rounds", type=int, default=1, metavar="N",
                    help="break, run the commands, continue, and break "
                         "again, N times. Two rounds say whether rip moved "
                         "between them, which one backtrace cannot.")
    ap.add_argument("--ddb-gap", type=float, default=10.0, metavar="SECONDS",
                    help="how long to let the guest run between rounds")
    ap.add_argument("--ddb-timeout", type=float, default=30.0,
                    help="seconds to wait for the db> prompt, and for each "
                         "command's output")
    ap.add_argument("--poke-after", type=int, default=25, metavar="SECONDS",
                    help="once the kernel has started, if nothing new is "
                         "printed for this long, type a marker command at "
                         "the console and watch for it coming back. 0 to "
                         "never poke.")
    ap.add_argument("--loader-timeout", type=int, default=45,
                    help="seconds to wait for the loader prompt after asking "
                         "for it, before giving up and booting as-is")
    ap.add_argument("--reboot-ok", action="store_true",
                    help="do not pass -no-reboot. A vm image built with a "
                         "swap partition has growfs_enable and /firstboot "
                         "set, so its first boot resizes the root filesystem "
                         "and reboots; -no-reboot makes QEMU exit there and "
                         "the login prompt never arrives.")
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
    ]
    if not args.reboot_ok:
        cmd += ["-no-reboot"]
    size = os.path.getsize(args.image)
    print(f"booting {args.image} ({size / 1e6:.0f} MB) as {args.target}")
    print("  " + " ".join(cmd) + "\n")

    # stdin is always a pipe now, not only when --run was given: the loader
    # menu has to be typed at before the kernel is started, and that happens
    # on every boot.
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            stdin=subprocess.PIPE)
    assert proc.stdout is not None
    os.set_blocking(proc.stdout.fileno(), False)

    started = time.time()
    buf = b""
    reached: list[str] = []
    verdict = None
    # Where the loader-menu conversation has got to.
    #   0  nothing seen yet
    #   1  asked for the loader prompt, waiting for it
    #   2  console set and boot sent, or given up on - either way, done
    loader_stage = 0 if args.loader_console else 2
    loader_asked_at = 0.0
    reboots = 0
    console_ok = False
    console_note = ""
    last_output = time.time()
    pokes = 0
    ddb_tried = False
    # Not a with-block: the interrogation phase below writes to the same
    # log, and closing it at the end of the boot loop made the first run of
    # this code die with "write to closed file".
    log = open(args.log, "wb")
    try:
        while time.time() - started < args.timeout:
            # The loader steering below has a timeout of its own, and a
            # loader that ignores the keypress sends nothing at all - so the
            # body of this loop cannot be reachable only when a chunk
            # arrives. The first version of it was, and the fallback that
            # exists precisely for the silent case could never fire.
            chunk = proc.stdout.read(4096)
            if chunk:
                log.write(chunk)
                log.flush()
                sys.stdout.write(chunk.decode("utf-8", "replace"))
                sys.stdout.flush()
                buf += chunk
                buf = buf[-65536:]
                last_output = time.time()
            vis = _plain(buf)

            # Phases before failures, so a panic during the kernel's own
            # startup still reports that the kernel started. The verdict is
            # the panic either way; `reached` is a separate fact and should
            # not be emptied by one.
            for name, pats in PHASES:
                if name not in reached and any(p.search(vis) for p in pats):
                    reached.append(name)
                    print(f"\n  [{name} reached]\n")

            for pat, why in FAILURE:
                m = pat.search(vis)
                if m:
                    verdict = ("FAIL", why,
                               m.group(0).decode("utf-8", "replace"))
                    if why == "could not mount root":
                        _explain_mountroot(vis)
                    break
            if verdict:
                break
            if "userland" in reached:
                verdict = ("OK", "reached userland", "")
                break

            # `set console` at the loader prompt is not persistent, and a
            # reboot starts a fresh loader that reads loader.conf again. The
            # vm image reboots on purpose: release/ gives it a swap
            # partition, so vmimage.subr sets growfs_enable and touches
            # /firstboot, and the first boot resizes the root filesystem and
            # restarts. Without re-steering, everything after that reboot
            # would go to Video and the run would end on a kernel that had
            # already booted twice.
            #
            # The buffer was cleared when `boot` was sent, so the menu can
            # only be in it again because a new one was drawn.
            if (loader_stage == 2 and args.loader_console
                    and "kernel" in reached and reboots < 3
                    and LOADER_MENU.search(vis)):
                reboots += 1
                print(f"\n  [back at the loader menu - reboot {reboots}; "
                      "steering the console again]\n")
                loader_stage = 0

            # Take the console off Video before the kernel starts, or
            # everything below is invisible. Nothing to do once the kernel
            # is talking, which is also the case where the image already
            # had a serial console configured.
            if loader_stage < 2 and ("kernel" not in reached or reboots):
                if loader_stage == 0 and LOADER_MENU.search(vis):
                    print("\n  [loader menu: escaping to the loader "
                          "prompt]\n")
                    _send(proc, "3")
                    loader_stage = 1
                    loader_asked_at = time.time()
                    # The menu's own text contains the word `boot` several
                    # times and the prompt banner is what is being waited
                    # for; start clean so the two cannot be confused.
                    buf = b""
                elif loader_stage == 1 and LOADER_READY.search(vis):
                    print(f"\n  [loader prompt: console="
                          f"{args.loader_console}]\n")
                    # Bounded by whichever runs out first. A steering wait
                    # longer than the run itself spends the whole timeout at
                    # the loader prompt and reports nothing about the kernel.
                    left = args.timeout - (time.time() - started)
                    console_ok, console_note = _steer_loader(
                        proc, log, args.loader_console,
                        max(2.0, min(args.loader_timeout, left - 5)),
                        extra=args.loader_set)
                    print(f"  [{console_note}]\n")
                    loader_stage = 2
                    buf = b""
                elif (loader_stage == 1 and
                      time.time() - loader_asked_at > args.loader_timeout):
                    # The prompt never came. Boot anyway rather than sitting
                    # here: an image that ignores the key is still an image
                    # worth watching, and the verdict below will say what
                    # was and was not seen.
                    print("\n  [no loader prompt within "
                          f"{args.loader_timeout}s; booting as-is]\n")
                    _send(proc, "\nboot\n")
                    loader_stage = 3

            # Nothing has been printed for a while and the kernel is up:
            # ask the console whether anything is listening. Three tries,
            # spaced, because the first may land while init is still
            # setting up its terminal.
            if (args.poke_after and "kernel" in reached
                    and "userland" not in reached and pokes < 3
                    and time.time() - last_output > args.poke_after):
                pokes += 1
                print(f"\n  [nothing for {args.poke_after}s; poking the "
                      f"console ({pokes}/3)]\n")
                _send(proc, f"\necho {ALIVE}\n")
                last_output = time.time()

            # The pokes are spent and nothing answered. Ask the kernel.
            if (args.ddb_on_hang and not ddb_tried
                    and "kernel" in reached and "userland" not in reached
                    and pokes >= 3
                    and time.time() - last_output > args.poke_after):
                ddb_tried = True
                print("\n  [console silent after three pokes; breaking to "
                      "the kernel debugger]\n")
                break_to_ddb(proc, log, args.ddb_timeout,
                             args.ddb_cmd or ["ps", "bt"],
                             rounds=args.ddb_rounds, gap=args.ddb_gap)
                last_output = time.time()

            if not chunk:
                if proc.poll() is not None:
                    break
                time.sleep(0.2)

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
    # What was typed at the loader, at the end where a tail can reach it. A
    # verbose boot is hundreds of lines and this used to be above all of
    # them, so "did that set actually land" was unanswerable from the log.
    if LOADER_SENT:
        print("    loader commands:")
        for cmd, ok, why in LOADER_SENT:
            print(f"      {'ok  ' if ok else 'FAIL'} {cmd}   ({why})")
    # And what the kernel debugger said, for the same reason.
    if DDB_REPORT:
        print("    kernel debugger:")
        for cmd, out in DDB_REPORT:
            if "\n" in out:
                print(f"      db> {cmd}")
                for line in out.splitlines():
                    print(f"        {line}")
            else:
                print(f"      db> {cmd}   {out}")
        _explain_ddb(DDB_REPORT,
                     _load_symbols(args.symbols) if args.symbols else None,
                     _load_disasm(args.disasm) if args.disasm else None)
    print(f"    phases reached: {', '.join(reached) or 'none'}")
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
    if "kernel" in reached:
        # "never reached userland" means this test never saw its marker on
        # the console. It does NOT mean no userland process ran, and run 43
        # is the case where saying so was wrong: the debugger frame put pid
        # 1 at a user rip. Say which of the two this is.
        user_rip = any(
            m and int(m.group(1), 16) < KERNBASE
            for m in (IFRAME.search(out) for _, out in DDB_REPORT))
        if user_rip:
            print(f"FAIL nothing reached the console in {args.timeout}s, and "
                  "the kernel debugger")
            print("     found pid 1 executing in USERLAND. So userland was "
                  "entered and")
            print("     is not talking - a hang or a fault loop INSIDE the "
                  "process, not a")
            print("     kernel hang and not a failure to exec. The 'what the "
                  "debugger")
            print("     output says' block above locates it.")
        else:
            print(f"FAIL the kernel started and never reached userland in "
                  f"{args.timeout}s.")
            print("     A hang after the kernel came up, not a boot failure. "
                  "The")
            print("     tail of the log is where it stopped.")
        return 1
    if "loader" in reached:
        print(f"FAIL the loader ran and the kernel never started in "
              f"{args.timeout}s.")
        print("     The loader menu drawing itself is not a boot. This is the")
        print("     case run 15 reported as success.")
        if args.loader_console and loader_stage == 2 and console_ok:
            print(f"     The console was set to {args.loader_console} at the")
            print("     loader prompt and the loader echoed the whole")
            print("     command back, so this is not the kernel printing")
            print("     somewhere else - it is the kernel not printing.")
        elif args.loader_console and loader_stage == 2:
            print(f"     The console was NOT confirmed: {console_note}.")
            print("     So the kernel may have booted to the Video console")
            print("     and be running unseen. This is not evidence about")
            print("     the kernel either way.")
        elif args.loader_console and loader_stage == 3:
            print("     The loader menu was answered and the loader prompt")
            print("     never came, so the console was never changed. The")
            print("     kernel may be running on the Video console, unseen.")
        elif args.loader_console:
            print("     The loader menu never appeared, so the console was")
            print("     never changed and the kernel may be running on the")
            print("     Video console, unseen. Check the tail of the log.")
        return 1
    print(f"FAIL nothing on the serial console in {args.timeout}s.")
    print("     The loader did not run, or the image is not bootable at all.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
