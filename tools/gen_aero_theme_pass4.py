#!/usr/bin/env python3
"""Generate Wave 3 pass-4 Aero theme SVG assets."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "theme" / "plasma"
ACCENT = "#5082c8"
ACCENT_DARK = "#2858a0"
GLASS = 'stop-color="#ffffff" stop-opacity="0.18"'


def glass_rect(w: int, h: int, rx: int = 4) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<svg width="{w}" height="{h}" viewBox="0 0 {w} {h}" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="g" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="{ACCENT}" stop-opacity="0.55"/>
      <stop offset="1" stop-color="{ACCENT_DARK}" stop-opacity="0.85"/>
    </linearGradient>
    <linearGradient id="shine" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#ffffff" stop-opacity="0.30"/>
      <stop offset="0.2" stop-color="#ffffff" stop-opacity="0.06"/>
      <stop offset="1" stop-color="#ffffff" stop-opacity="0"/>
    </linearGradient>
  </defs>
  <rect x="0" y="0" width="{w}" height="{h}" rx="{rx}" fill="url(#g)" stroke="{ACCENT_DARK}" stroke-width="1"/>
  <rect x="1" y="1" width="{w-2}" height="{min(h//3, 24)}" rx="{max(rx-1,0)}" fill="url(#shine)"/>
</svg>
"""


def icon_tile(name: str, label: str, w: int = 32, h: int = 32) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<svg width="{w}" height="{h}" viewBox="0 0 {w} {h}" xmlns="http://www.w3.org/2000/svg">
  <rect x="1" y="1" width="{w-2}" height="{h-2}" rx="3" fill="{ACCENT}" fill-opacity="0.35" stroke="{ACCENT}" stroke-opacity="0.7"/>
  <text x="{w//2}" y="{h//2+4}" text-anchor="middle" font-family="Segoe UI,sans-serif" font-size="8" fill="#ffffff" fill-opacity="0.9">{label[:3]}</text>
</svg>
"""


PANEL_ICONS = {
    "music": "♪", "videos": "▶", "games": "🎮", "computer": "PC", "network": "Net",
    "devices": "Dev", "favorites": "★", "recent": "Rc", "all-programs": "All",
    "power-button": "⏻", "lock": "🔒", "sleep": "Zzz", "restart": "↻", "logoff": "→",
    "help": "?", "settings": "⚙", "run": "Run", "search": "🔍",
}

WIDGETS = [
    ("combobox", 120, 24), ("listbox", 160, 120), ("treeview", 180, 140),
    ("header", 200, 28), ("footer", 200, 24), ("badge", 20, 20),
    ("pin", 16, 16), ("volume", 24, 24), ("battery", 28, 14),
    ("wifi", 24, 24), ("bluetooth", 24, 24), ("dropdown", 120, 24),
    ("editbox", 160, 24), ("groupbox", 200, 100), ("splitter", 8, 120),
]

DIALOGS = [
    ("progress", 320, 80), ("wizard", 480, 360), ("properties", 400, 320),
    ("about", 360, 280), ("inputbox", 320, 120), ("conflict", 400, 200),
]

DECORATION = [
    ("border-top", 200, 8), ("border-right", 8, 200), ("border-bottom", 200, 8),
    ("resize-grip", 12, 12), ("glow-active", 200, 36), ("glow-inactive", 200, 36),
    ("caption-icon", 16, 16),
]


def main() -> None:
    created: list[str] = []

    for name, label in PANEL_ICONS.items():
        path = ROOT / "panel" / f"{name}.svg"
        path.write_text(icon_tile(name, label, 32, 32), encoding="utf-8")
        created.append(path.relative_to(ROOT.parent.parent).as_posix())

    for name, w, h in WIDGETS:
        path = ROOT / "widgets" / f"{name}.svg"
        path.write_text(glass_rect(w, h), encoding="utf-8")
        created.append(path.relative_to(ROOT.parent.parent).as_posix())

    for name, w, h in DIALOGS:
        path = ROOT / "dialogs" / f"{name}.svg"
        path.write_text(glass_rect(w, h, rx=6), encoding="utf-8")
        created.append(path.relative_to(ROOT.parent.parent).as_posix())

    for name, w, h in DECORATION:
        path = ROOT / "decoration" / f"{name}.svg"
        path.write_text(glass_rect(w, h, rx=2 if "border" in name else 4), encoding="utf-8")
        created.append(path.relative_to(ROOT.parent.parent).as_posix())

    print(f"Created {len(created)} SVG assets")
    for c in created:
        print(c)


if __name__ == "__main__":
    main()
