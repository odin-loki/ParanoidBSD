# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Buffered proposals.jsonl writer — avoid per-site open() on OneDrive."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "migration" / "clang_port"
PATH = OUT / "proposals.jsonl"

_buf: list[str] = []
_MAX_BUF = 256


def reset() -> None:
    _buf.clear()


def propose(file: str, kind: str, payload: dict) -> None:
    _buf.append(json.dumps({"file": file, "kind": kind, **payload}))
    if len(_buf) >= _MAX_BUF:
        flush()


def flush() -> None:
    if not _buf:
        return
    OUT.mkdir(parents=True, exist_ok=True)
    with PATH.open("a", encoding="utf-8") as f:
        f.write("\n".join(_buf) + "\n")
    _buf.clear()
