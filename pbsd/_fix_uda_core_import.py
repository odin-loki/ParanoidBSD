#!/usr/bin/env python3
"""Insert `import pbsd.core;` into UDA descriptor modules that use Status without it."""
from pathlib import Path

root = Path(__file__).resolve().parent / "uda" / "descriptors"
fixed = 0
for path in sorted(root.glob("*.cppm")):
    text = path.read_text(encoding="utf-8")
    if "import pbsd.core" in text:
        continue
    if "Status" not in text and "status_err" not in text:
        continue
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    inserted = False
    i = 0
    while i < len(lines):
        out.append(lines[i])
        if not inserted and lines[i].startswith("export module "):
            # Skip one following blank line if present, then insert import.
            if i + 1 < len(lines) and lines[i + 1].strip() == "":
                i += 1
                out.append(lines[i])
            out.append("import pbsd.core;\n")
            if i + 1 >= len(lines) or lines[i + 1].strip() != "":
                out.append("\n")
            inserted = True
        i += 1
    if inserted:
        path.write_text("".join(out), encoding="utf-8", newline="\n")
        fixed += 1
        print(path.name)
print(f"FIXED={fixed}")
