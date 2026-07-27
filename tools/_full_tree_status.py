#!/usr/bin/env python3
"""Write combined full-tree status into todo_pass_report.md (append section)."""
from __future__ import annotations

import json
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "docs" / "migration" / "clang_port"
STAGED = OUT / "staged"

n_cpp = len(list(STAGED.rglob("*.cpp"))) if STAGED.exists() else 0
n_prop = 0
kinds: Counter[str] = Counter()
prop = OUT / "proposals.jsonl"
if prop.exists():
    for line in prop.open(encoding="utf-8"):
        line = line.strip()
        if not line:
            continue
        try:
            kinds[json.loads(line).get("kind", "?")] += 1
            n_prop += 1
        except json.JSONDecodeError:
            pass

n_ref = 0
ref = OUT / "refusals.jsonl"
if ref.exists():
    n_ref = sum(1 for line in ref.open(encoding="utf-8") if line.strip())

now = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
lines = [
    "",
    "## Full-tree status (combined)",
    "",
    f"Generated: `{now}`",
    "",
    f"- Staged `.cpp` files: **{n_cpp}** (target userland ~4028)",
    f"- Proposals: **{n_prop}**",
    f"- Refusals: **{n_ref}**",
    f"- Notes: first pass reached ~3700/4028 then resumed with `--skip`; "
    f"`ppp/main.c` hit 90s timeout once; buffered proposals + per-file timeout added.",
    "",
    "### Top proposal kinds",
    "",
    "| Kind | Count |",
    "|---|---:|",
]
for k, v in kinds.most_common(25):
    lines.append(f"| `{k}` | {v} |")
lines.append("")
(OUT / "full_tree_status.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"staged={n_cpp} proposals={n_prop} refusals={n_ref}")
print("wrote", OUT / "full_tree_status.md")
