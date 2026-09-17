# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Offline checks for routing, parsers, and append-only session prefix."""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))

from pbsd_agent.esbmc_check import (
    CHECKS_OFF,
    _reject_weakening,
    build_cmd,
    parse_esbmc_output,
)
import pbsd_agent.esbmc_check as esbmc_mod
from pbsd_agent.session import FileContext, FileSession, parse_agent_payload, starting_tier


def test_routing() -> None:
    echo = FileContext("hbsd/src/bin/echo/echo.c", "x", None, "", [], 3, "C")
    assert starting_tier(echo) == 1
    echo.risk_tier = 1
    assert starting_tier(echo) == 2
    hard = FileContext("hbsd/src/sys/vm/uma/uma_core.c", "x", None, "", [], 3, "C")
    assert starting_tier(hard) == 2


def test_parse_and_esbmc() -> None:
    p = parse_agent_payload('{"spec_notes": "s", "port_cppm": "mod"}')
    assert p["port_cppm"] == "mod"
    status, sat, cex = parse_esbmc_output("VERIFICATION FAILED\nCounterexample\nfoo")
    assert status == "failed" and sat == "SAT" and "Counterexample" in cex


def test_silence_is_not_a_proof() -> None:
    """Output with no verdict line is `error'. It was once `ok' whenever the
    exit status was zero, which turns an unreadable run into a clean one."""
    for blob in ("", "   \n", "esbmc: parsing something, then nothing"):
        status, sat, _ = parse_esbmc_output(blob)
        assert status == "error", blob
        assert sat != "UNSAT", blob


def test_a_check_cannot_be_switched_off_silently() -> None:
    """ESBMC has bounds, pointer and division checks ON by default. This
    wrapper passed --no-bounds-check unconditionally and so reported clean
    verdicts for code whose array accesses nothing had checked."""
    for flag in CHECKS_OFF:
        try:
            _reject_weakening([flag])
        except ValueError:
            pass
        else:
            raise AssertionError(f"{flag} was accepted")
    # And the same flag smuggled in with a value.
    try:
        _reject_weakening(["--no-bounds-check=1"])
    except ValueError:
        pass
    else:
        raise AssertionError("--no-bounds-check=1 was accepted")
    _reject_weakening(["--overflow-check", "--unwind=4"])  # must not raise


def test_default_flags_do_not_disable_a_check() -> None:
    """The command the wrapper builds, read back WITHOUT a binary present.

    Reached through build_cmd rather than through esbmc_check, because on
    a machine with no esbmc the latter returns `skipped' and a test that
    asserts about a command that was never built passes vacuously - which
    is how the first version of this test passed while the flag it was
    meant to catch was still there."""
    cmd = build_cmd("esbmc", Path("/x.c"))
    for flag in CHECKS_OFF:
        assert flag not in cmd, f"{flag} is in the default command line"
    assert "--overflow-check" in cmd, "overflow is OFF in ESBMC by default"
    assert any(a.startswith("--unwind") for a in cmd)


def test_an_unreadable_run_is_not_reported_clean() -> None:
    """The promotion lived in esbmc_check, not in parse_esbmc_output, so
    it has to be tested THROUGH esbmc_check with a fake binary and a fake
    subprocess. Testing the parser alone leaves the bug reachable."""
    import subprocess as _sp

    class _Proc:
        returncode = 0
        stdout = "esbmc: started, then said nothing a parser can read\n"
        stderr = ""

    real_which, real_run = esbmc_mod.shutil.which, esbmc_mod.subprocess.run
    try:
        esbmc_mod.shutil.which = lambda _n: "/fake/esbmc"
        esbmc_mod.subprocess.run = lambda *a, **k: _Proc()
        res = esbmc_mod.esbmc_check(Path("/x.c"))
    finally:
        esbmc_mod.shutil.which, esbmc_mod.subprocess.run = real_which, real_run
    assert res.status == "error", f"exit 0 with no verdict became {res.status}"
    assert res.sat != "UNSAT", "silence was reported as a proof"
    assert _sp is not None


def test_prefix_frozen() -> None:
    ctx = FileContext("hbsd/src/bin/echo/echo.c", "int x;", None, "stub", [], 3, "C")
    sess = FileSession(ctx, do_diff=False, do_ir=False, do_asan=False, do_esbmc=False)
    assert sess.prefix_is_frozen()
    frozen = [dict(m) for m in sess.messages[: sess._frozen_prefix]]
    sess.messages.append({"role": "assistant", "content": "draft"})
    sess.messages.append({"role": "user", "content": "fix"})
    assert sess.messages[: sess._frozen_prefix] == frozen


if __name__ == "__main__":
    test_routing()
    test_parse_and_esbmc()
    test_silence_is_not_a_proof()
    test_a_check_cannot_be_switched_off_silently()
    test_default_flags_do_not_disable_a_check()
    test_an_unreadable_run_is_not_reported_clean()
    test_prefix_frozen()
    print("ok")
