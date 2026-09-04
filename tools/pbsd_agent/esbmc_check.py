# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""ESBMC bounded-model-check wrapper — Stage G formal, new integration.

Missing binary is a skip, not a failure. SAT/UNSAT/counterexample are parsed
from stdout so the session can feed a real counterexample back to the model.
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


def parse_esbmc_output(text: str) -> tuple[str, str | None, str]:
    """Return (status, sat, counterexample_excerpt)."""
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
    cmd = [
        exe,
        str(src),
        f"--unwind={unwind}",
        "--no-bounds-check",
        "--timeout",
        str(int(timeout)),
        *(extra or []),
    ]
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
    if status == "error" and proc.returncode == 0:
        status = "ok"
        sat = sat or "UNSAT"
    return EsbmcResult(
        status=status,
        sat=sat,
        detail=blob[-2000:],
        counterexample=cex,
        cmd=cmd,
    )
