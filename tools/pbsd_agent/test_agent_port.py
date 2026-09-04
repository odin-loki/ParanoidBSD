# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Offline checks for routing, parsers, and append-only session prefix."""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))

from pbsd_agent.esbmc_check import parse_esbmc_output
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
    test_prefix_frozen()
    print("ok")
