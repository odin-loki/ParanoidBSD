#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Install GitHub CodeQL CLI onto PATH. Durable enough to keep."""
from __future__ import annotations

import json
import os
import shutil
import ssl
import sys
import tarfile
import tempfile
import urllib.request
import zipfile
from pathlib import Path

DEST = Path(os.environ.get("CODEQL_HOME", str(Path.home() / ".local" / "codeql")))
BIN = Path.home() / ".local" / "bin"
API = "https://api.github.com/repos/github/codeql-cli-binaries/releases/latest"


def _chmod_executables(root: Path) -> int:
    """zipfile drops execute bits. Restore them on ELF and shebang files."""
    n = 0
    for p in root.rglob("*"):
        if not p.is_file():
            continue
        try:
            with p.open("rb") as fh:
                magic = fh.read(4)
        except OSError:
            continue
        if magic[:2] == b"#!" or magic[:4] == b"\x7fELF":
            mode = p.stat().st_mode
            if mode & 0o111 != 0o111:
                p.chmod(mode | 0o755)
                n += 1
    return n


def main() -> int:
    BIN.mkdir(parents=True, exist_ok=True)
    existing = shutil.which("codeql")
    if existing:
        print("already on PATH:", existing)
        os.execvp("codeql", ["codeql", "version"])

    ctx = ssl.create_default_context()
    req = urllib.request.Request(API, headers={"User-Agent": "paranoidbsd-verify"})
    with urllib.request.urlopen(req, context=ctx, timeout=60) as r:
        rel = json.loads(r.read().decode())
    tag = rel.get("tag_name")
    print("latest", tag)
    asset = None
    for a in rel.get("assets", []):
        name = a.get("name", "")
        if name in ("codeql-linux64.tar.gz", "codeql-linux64.zip"):
            asset = a
            break
    if not asset:
        names = [a.get("name") for a in rel.get("assets", [])]
        print("no linux64 asset in", names, file=sys.stderr)
        return 1
    url = asset["browser_download_url"]
    name = asset["name"]
    print("download", name, asset.get("size"), url, flush=True)
    tmp = Path(tempfile.mkdtemp(prefix="codeql-"))
    archive = tmp / name
    urllib.request.urlretrieve(url, archive)
    print("saved", archive, archive.stat().st_size, flush=True)

    if DEST.exists():
        shutil.rmtree(DEST)
    DEST.parent.mkdir(parents=True, exist_ok=True)
    if name.endswith(".zip"):
        with zipfile.ZipFile(archive) as z:
            z.extractall(tmp / "ex")
    else:
        with tarfile.open(archive, "r:gz") as t:
            t.extractall(tmp / "ex")
    # archive contains a top-level codeql/ directory
    extracted = tmp / "ex"
    cand = list(extracted.glob("**/codeql"))
    # the launcher is a file named codeql
    launcher = None
    for p in cand:
        if p.is_file():
            launcher = p
            break
    if launcher is None:
        print("no codeql launcher under", extracted, file=sys.stderr)
        print(list(extracted.iterdir())[:20], file=sys.stderr)
        return 1
    src_dir = launcher.parent
    shutil.move(str(src_dir), str(DEST))
    _chmod_executables(DEST)
    launcher = DEST / "codeql"
    launcher.chmod(launcher.stat().st_mode | 0o755)
    link = BIN / "codeql"
    if link.exists() or link.is_symlink():
        link.unlink()
    link.symlink_to(DEST / "codeql")
    print("installed", DEST, "->", link, flush=True)
    env = os.environ.copy()
    env["PATH"] = str(BIN) + ":" + env.get("PATH", "")
    os.execve(str(link), ["codeql", "version"], env)


if __name__ == "__main__":
    raise SystemExit(main())
