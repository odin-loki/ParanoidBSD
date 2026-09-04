#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
import re
from pathlib import Path

PBSD = Path(__file__).resolve().parents[1] / "pbsd"
CMAKE = (PBSD / "CMakeLists.txt").read_text(encoding="utf-8")

def has_target(tgt: str) -> bool:
    return (
        f"if(NOT TARGET {tgt})" in CMAKE
        or re.search(rf"add_library\({re.escape(tgt)}\)", CMAKE) is not None
    )

issues = []
for area in ("net", "fs", "geom", "zfs"):
    agg = (PBSD / area / f"pbsd.{area}.cppm").read_text(encoding="utf-8")
    exports = re.findall(rf"export import pbsd\.{area}\.([^;]+);", agg)
    m = re.search(rf"target_link_libraries\(pbsd_{area} PUBLIC ([^\)]+)\)", CMAKE, re.DOTALL)
    link = m.group(1) if m else ""
    for imp in exports:
        tgt = f"pbsd_{area}_{imp.replace('.', '_')}"
        if not has_target(tgt):
            issues.append(f"{area}: export {imp} -> missing cmake target {tgt}")
        elif not re.search(rf"\b{re.escape(tgt)}\b", link):
            issues.append(f"{area}: export {imp} -> {tgt} not in aggregate link")

print(f"issues: {len(issues)}")
for i in issues:
    print(i)
