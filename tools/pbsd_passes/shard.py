# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Per-process output shards, so the pipeline can run in parallel.

proposals.jsonl and the tier-3 side files are each opened in append mode once
per source file. That is fine for one process and unsafe for several: two
writes longer than the pipe buffer can interleave and leave a half-line, which
would corrupt the record silently rather than fail loudly.

Under `--jobs` the parent sets PBSD_SHARD=1 before the pool starts. Every
worker then appends to a file named after its own pid, and the parent
concatenates the shards once the pool has drained. With one job nothing
changes and the paths are exactly what they always were.
"""
from __future__ import annotations

import os
from pathlib import Path

ENV = "PBSD_SHARD"


def sharding() -> bool:
    return os.environ.get(ENV) == "1"


def shard_path(base: Path) -> Path:
    """The file this process should append to."""
    if not sharding():
        return base
    return base.with_name(f"{base.stem}.{os.getpid()}{base.suffix}")


def merge_shards(base: Path) -> int:
    """Fold every shard of `base` back into it. Returns the lines merged."""
    shards = sorted(base.parent.glob(f"{base.stem}.*{base.suffix}"))
    merged = 0
    if not shards:
        return 0
    with base.open("a", encoding="utf-8") as out:
        for shard in shards:
            try:
                text = shard.read_text(encoding="utf-8")
            except OSError:
                continue
            if text and not text.endswith("\n"):
                text += "\n"
            out.write(text)
            merged += text.count("\n")
            shard.unlink(missing_ok=True)
    return merged
