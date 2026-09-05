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
        print(f"FAIL the kernel started and never reached userland in "
              f"{args.timeout}s.")
        print("     A hang after the kernel came up, not a boot failure. The")
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
