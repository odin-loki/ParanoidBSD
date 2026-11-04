#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""PBSD migration driver — DeepSeek Flash → Pro agent port (single path).

    python pbsd.py --status
    python pbsd.py --dry-run --scope bin
    python pbsd.py --scope bin,usr.bin
    python pbsd.py --self-test

All model work goes to DeepSeek via tools/pbsd_agent_port.py.
No Cursor API. No mechanical IR sweep.
"""
from __future__ import annotations

import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TOOLS = ROOT / "tools"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from convert_c_batch import load_progress  # noqa: E402
from pbsd_agent_port import main as agent_main  # noqa: E402


def print_status() -> int:
    progress = load_progress()
    entries = progress.get("entries") or []
    counts = Counter(e.get("status", "?") for e in entries)
    print(f"batch_progress entries={len(entries)} updated={progress.get('updated', '?')}")
    for status, n in sorted(counts.items(), key=lambda kv: (-kv[1], kv[0])):
        print(f"  {status}: {n}")
    waves = progress.get("waves") or {}
    if waves:
        print("waves:")
        for name, w in sorted(waves.items()):
            print(
                f"  {name}: total={w.get('total', 0)} "
                f"converted={w.get('converted', 0)} stubbed={w.get('stubbed', 0)}"
            )
    print("failures: docs/migration/clang_port/agent_port_failures.jsonl")
    print("cost_log: docs/migration/clang_port/agent_port_cost.jsonl")
    return 0


def main(argv: list[str] | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "--status":
        return print_status()
    # Allow `pbsd.py status` as well as `--status`
    if argv == ["status"]:
        return print_status()
    return agent_main(argv)


if __name__ == "__main__":
    # argparse lives in agent_main; keep a tiny top-level help for --status
    if len(sys.argv) > 1 and sys.argv[1] in {"-h", "--help"}:
        print(__doc__)
        print("Agent-port flags (passed through):")
        raise SystemExit(agent_main(["--help"]))
    raise SystemExit(main())
