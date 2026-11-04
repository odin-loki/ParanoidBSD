#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The semantic CFLAGS the real build gives a file.

The oracle compiles both sides with its own flags, which makes the
comparison fair but not necessarily relevant. lib/msun/Makefile sets

    CFLAGS+=	-ffp-exception-behavior=maytrap
    CFLAGS+=	-fno-math-errno

and those are not incidental to a math library, they are its subject
matter. A port proved equivalent under default flags has been proved
equivalent to something the tree does not build.

Flags are read out of the nearest Makefile rather than out of bmake,
because bmake needs the whole build environment to answer and the answer
is wanted on Linux too. So this is deliberately partial:

  * literal `-f`, `-m` and `-W` flags are taken;
  * anything containing `${` is skipped - resolving it needs bmake;
  * `-I` and `-D` are skipped, because the oracle already builds its own
    include environment and mixing the two produces worse errors than
    either alone;
  * per-file `CFLAGS.<name>+=` is applied when the name matches.

What it cannot resolve, it says so rather than guessing.
"""

from __future__ import annotations

import re
from pathlib import Path

CFLAGS_LINE = re.compile(r"^\s*CFLAGS(?:\.(\S+?))?\s*\+?=\s*(.*)$")
TAKE = ("-f", "-m", "-W", "-std=", "-O")
SKIP_EXACT = {"-Werror", "-Wall", "-Wextra"}


def _literal_flags(value: str) -> list[str]:
    out = []
    for tok in value.split():
        if "${" in tok or "$(" in tok:
            continue
        if tok in SKIP_EXACT:
            continue
        if tok.startswith(TAKE):
            out.append(tok)
    return out


def flags_for(source: Path, stop_at: Path | None = None) -> tuple[list[str], list[str]]:
    """Return (flags, unresolved) for one source file.

    `unresolved` holds the CFLAGS entries that needed bmake, so a caller can
    report how complete the answer is instead of assuming it is complete.
    """
    source = Path(source)
    flags: list[str] = []
    unresolved: list[str] = []
    d = source.parent
    seen = 0
    while seen < 4:
        mk = d / "Makefile"
        if mk.is_file():
            seen += 1
            try:
                lines = mk.read_text(encoding="utf-8", errors="replace").splitlines()
            except OSError:
                lines = []
            i = 0
            while i < len(lines):
                m = CFLAGS_LINE.match(lines[i])
                if not m:
                    i += 1
                    continue
                per_file, value = m.group(1), m.group(2)
                while value.endswith("\\") and i + 1 < len(lines):
                    i += 1
                    value = value[:-1] + " " + lines[i].strip()
                if per_file and per_file not in (source.name,
                                                 source.with_suffix(".c").name):
                    i += 1
                    continue
                for tok in value.split():
                    if "${" in tok or "$(" in tok:
                        unresolved.append(tok)
                flags.extend(_literal_flags(value))
                i += 1
        if stop_at is not None and d == stop_at:
            break
        if d.parent == d:
            break
        d = d.parent
    # Later (deeper) files were read first; keep first occurrence.
    deduped = list(dict.fromkeys(flags))
    return deduped, list(dict.fromkeys(unresolved))


if __name__ == "__main__":
    import sys
    for arg in sys.argv[1:]:
        f, u = flags_for(Path(arg))
        print(f"{arg}")
        print(f"  flags      : {' '.join(f) or '(none)'}")
        print(f"  needs bmake: {' '.join(u) or '(none)'}")
