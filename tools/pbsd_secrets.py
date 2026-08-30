"""Load KEY=value files from secrets/ into os.environ.

Canonical file: <repo>/secrets/api-keys (gitignored).
Existing non-empty environment variables win so a shell override still works.
Empty values in the file are ignored.
"""
from __future__ import annotations

import os
from pathlib import Path

WINDOWS_CHECKOUT = Path("/mnt/c/Users/odinl/OneDrive/Desktop/Operating System")


def parse_secrets_text(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for raw in text.splitlines():
        line = raw.strip().lstrip("\ufeff")
        if not line or line.startswith("#"):
            continue
        if line.startswith("export "):
            line = line[7:].strip()
        if "=" not in line:
            continue
        key, _, val = line.partition("=")
        key = key.strip()
        val = val.strip()
        if len(val) >= 2 and val[0] == val[-1] and val[0] in "\"'":
            val = val[1:-1]
        if key and val:
            out[key] = val
    return out


def secret_file_candidates(root: Path | None = None) -> list[Path]:
    roots: list[Path] = []
    explicit = os.environ.get("PBSD_SECRETS", "").strip()
    if explicit:
        return [Path(explicit)]
    if root is not None:
        roots.append(root)
    here = Path(__file__).resolve()
    if here.parent.name == "tools":
        roots.append(here.parents[1])
    if WINDOWS_CHECKOUT.is_dir():
        roots.append(WINDOWS_CHECKOUT)
    home_pbsd = Path.home() / "pbsd"
    if home_pbsd.is_dir():
        roots.append(home_pbsd)
    seen: set[Path] = set()
    files: list[Path] = []
    for r in roots:
        r = r.resolve()
        if r in seen:
            continue
        seen.add(r)
        files.append(r / "secrets" / "api-keys")
        files.append(r / "secrets" / "api-keys.env")
    return files


def load_secrets(root: Path | None = None, *, overwrite: bool = False) -> Path | None:
    """Apply the first readable secrets file. Returns the path used, or None."""
    for path in secret_file_candidates(root):
        if not path.is_file():
            continue
        try:
            parsed = parse_secrets_text(path.read_text(encoding="utf-8", errors="replace"))
        except OSError:
            continue
        for key, val in parsed.items():
            if overwrite or not os.environ.get(key):
                os.environ[key] = val
        return path
    return None


def loaded_key_names() -> list[str]:
    names = ("DEEPSEEK_API_KEY", "CURSOR_API_KEY")
    return [n for n in names if os.environ.get(n)]
