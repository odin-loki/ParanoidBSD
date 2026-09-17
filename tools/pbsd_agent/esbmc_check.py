# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""ESBMC bounded-model-check wrapper — Stage G formal, new integration.

Missing binary is a skip, not a failure. SAT/UNSAT/counterexample are parsed
from stdout so the session can feed a real counterexample back to the model.

Two rules this wrapper got wrong once, both the same rule
--------------------------------------------------------
ESBMC's check polarity is the OPPOSITE of CBMC's. Bounds, pointer and
division checks are ON by default and are switched off by a `--no-*'
flag; overflow and leak checks are OFF and must be named. This wrapper
passed `--no-bounds-check' unconditionally, so every ESBMC run it ever
made had array bounds checking disabled and reported a clean verdict for
code it had not checked. CHECKS_OFF below is a deny list and
`_reject_weakening' raises on any of them, including via `extra', because
a disabled check is invisible in the output that reports the result.

And an output with no verdict line in it is not a proof. The wrapper used
to promote an unparseable run to `ok/UNSAT' whenever the exit code was
zero, which manufactures a clean result out of silence — exactly the
failure the rest of this tree exists to prevent. An unreadable output is
now `error', and an empty one is `error' too.
"""
from __future__ import annotations

import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass
class EsbmcResult:
    status: str  # ok | failed | skipped | timeout | error
    sat: str | None = None  # UNSAT | SAT | None
    detail: str = ""
    counterexample: str = ""
    cmd: list[str] | None = None


def find_esbmc() -> str | None:
    return shutil.which("esbmc")


# Flags that turn a check OFF. Passing one of these means the run does not
# check what its own clean verdict appears to claim.
CHECKS_OFF = (
    "--no-bounds-check",
    "--no-pointer-check",
    "--no-div-by-zero-check",
    "--no-assertions",
    "--no-pointer-relation-check",
    "--no-align-check",
    "--no-unwinding-assertions",
)


def _reject_weakening(flags: list[str]) -> None:
    """A caller may add flags; it may not switch off a check silently."""
    for f in flags:
        head = f.split("=", 1)[0]
        if head in CHECKS_OFF:
            raise ValueError(
                f"{head} disables a check whose absence would be invisible "
                "in the verdict; see CHECKS_OFF in this module")


def parse_esbmc_output(text: str) -> tuple[str, str | None, str]:
    """Return (status, sat, counterexample_excerpt).

    A text with no verdict line is `error'. It is never `ok'.
    """
    if not text.strip():
        return "error", None, ""
    upper = text.upper()
    cex = ""
    if "COUNTEREXAMPLE" in upper:
        idx = upper.find("COUNTEREXAMPLE")
        cex = text[idx : idx + 2500]
    if "VERIFICATION SUCCESSFUL" in upper or "UNSATISFIABLE" in upper:
        return "ok", "UNSAT", cex
    if "VERIFICATION FAILED" in upper or "SATISFIABLE" in upper:
        return "failed", "SAT", cex
    if "PARSING" in upper and "ERROR" in upper:
        return "error", None, cex
    return "error", None, cex


def build_cmd(
    exe: str,
    src: Path,
    *,
    unwind: int = 8,
    timeout: float = 60.0,
    extra: list[str] | None = None,
) -> list[str]:
    """The command line, built where a test can read it without a binary.

    This is a separate function for exactly one reason: a test that can
    only reach the flags by RUNNING esbmc cannot run at all on a machine
    without esbmc, and a test that cannot run is indistinguishable from a
    test that passes.
    """
    extra = list(extra or [])
    _reject_weakening(extra)
    return [
        exe,
        str(src),
        f"--unwind={unwind}",
        # Named because ESBMC leaves these OFF. Bounds, pointer and
        # division checks are on by default and are deliberately not
        # touched here: see CHECKS_OFF.
        "--overflow-check",
        "--memory-leak-check",
        "--timeout",
        str(int(timeout)),
        *extra,
    ]


def esbmc_check(
    src: Path,
    *,
    timeout: float = 60.0,
    unwind: int = 8,
    extra: list[str] | None = None,
) -> EsbmcResult:
    exe = find_esbmc()
    if exe is None:
        return EsbmcResult(
            status="skipped",
            detail="esbmc not on PATH — formal check skipped (tier3_maths still runs in deterministic passes)",
        )
    cmd = build_cmd(exe, src, unwind=unwind, timeout=timeout, extra=extra)
    try:
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout + 5,
        )
    except subprocess.TimeoutExpired:
        return EsbmcResult(status="timeout", detail=f"esbmc exceeded {timeout}s", cmd=cmd)
    except OSError as e:
        return EsbmcResult(status="error", detail=str(e), cmd=cmd)

    blob = (proc.stdout or "") + "\n" + (proc.stderr or "")
    status, sat, cex = parse_esbmc_output(blob)
    # NOT promoted to "ok" on a zero exit status. A run whose output holds
    # no verdict line was not read, and "we could not read it" and "it is
    # clean" are the two things this whole tree keeps apart.
    return EsbmcResult(
        status=status,
        sat=sat,
        detail=blob[-2000:],
        counterexample=cex,
        cmd=cmd,
    )
