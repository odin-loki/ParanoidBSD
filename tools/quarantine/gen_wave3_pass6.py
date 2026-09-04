#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 3 pass-6 KDE + Aero theme C++23 modules (Aero glass expansion)."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
KDE = ROOT / "pbsd" / "kde"
THEME = ROOT / "pbsd" / "theme" / "plasma"
CMAKE_KDE = ROOT / "pbsd" / "kde" / "CMakeLists.txt"
CMAKE_THEME = ROOT / "pbsd" / "theme" / "CMakeLists.txt"


def simple_module(
    mod: str,
    ns: str,
    upstream: str,
    doc: str,
    body_lines: list[str],
    extra_imports: list[str] | None = None,
) -> str:
    imports = ["import pbsd.core;"]
    if extra_imports:
        imports.extend(extra_imports)
    body = "\n".join(f"    {line}" for line in body_lines)
    return f"""export module {mod};

{chr(10).join(imports)}

/// Wave 3 pass 6 — {doc}
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


def theme_module(
    rel: str,
    mod: str,
    ns: str,
    imports: list[str],
    body: list[str],
) -> str:
    body_text = "\n".join(f"    {line}" for line in body)
    imports_text = "\n".join(imports)
    return f"""export module {mod};

import pbsd.core;
{imports_text}

/// Wave 3 pass 6 — PBSD Aero theme constants ({rel}).
export namespace {ns} {{

{body_text}

}} // namespace {ns}
"""


PLASMA: list[tuple[str, str, str, str, str, list[str], list[str] | None]] = [
    (
        "plasma.dialogs.cppm",
        "pbsd.kde.plasma.dialogs",
        "pbsd::kde::plasma::dialogs",
        "pbsd/theme/plasma/theme-colors.json",
        "Plasma dialog chrome constants (Aero glass dialogs).",
        [
            "enum class Kind : unsigned char {",
            "    MessageBox, FileDialog, Progress, Wizard, Properties, About, InputBox, Conflict",
            "};",
            'inline constexpr const char kMessageBoxSvg[] = "plasma/dialogs/messagebox.svg";',
            'inline constexpr const char kFileDialogSvg[] = "plasma/dialogs/filedialog.svg";',
            'inline constexpr const char kProgressSvg[] = "plasma/dialogs/progress.svg";',
            'inline constexpr const char kWizardSvg[] = "plasma/dialogs/wizard.svg";',
            'inline constexpr const char kPropertiesSvg[] = "plasma/dialogs/properties.svg";',
            'inline constexpr const char kAboutSvg[] = "plasma/dialogs/about.svg";',
            'inline constexpr const char kInputBoxSvg[] = "plasma/dialogs/inputbox.svg";',
            'inline constexpr const char kConflictSvg[] = "plasma/dialogs/conflict.svg";',
            "inline constexpr float kDefaultOpacity{0.78f};",
            "inline constexpr int kDefaultCornerRadius{8};",
            "inline constexpr int kTitleBarHeight{36};",
        ],
        ["import pbsd.kde.plasma.aero;"],
    ),
    (
        "plasma.widgets.cppm",
        "pbsd.kde.plasma.widgets",
        "pbsd::kde::plasma::widgets",
        "pbsd/theme/plasma/theme-colors.json",
        "Plasma widget SVG asset ids (Aero controls).",
        [
            "enum class Asset : unsigned char {",
            "    Background, Button, Frame, Notification, Slider, Scrollbar, Tab, Checkbox,",
            "    Radiobutton, Tasks, Tooltip, MenuItem, Progress, Spinner, HoverHighlight,",
            "    Combobox, Listbox, Treeview, Header, Footer, Badge, Pin, Volume, Battery,",
            "    Wifi, Bluetooth, Dropdown, Editbox, Groupbox, Splitter",
            "};",
            'inline constexpr const char kTabSvg[] = "plasma/widgets/tab.svg";',
            'inline constexpr const char kCheckboxSvg[] = "plasma/widgets/checkbox.svg";',
            'inline constexpr const char kRadiobuttonSvg[] = "plasma/widgets/radiobutton.svg";',
            'inline constexpr const char kMenuItemSvg[] = "plasma/widgets/menuitem.svg";',
            'inline constexpr const char kProgressSvg[] = "plasma/widgets/progress.svg";',
            'inline constexpr const char kSpinnerSvg[] = "plasma/widgets/spinner.svg";',
            'inline constexpr const char kHoverHighlightSvg[] = "plasma/widgets/hover-highlight.svg";',
            'inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";',
            "inline constexpr int kDefaultIconSize{22};",
            "inline constexpr int kDefaultFrameRadius{4};",
        ],
        None,
    ),
    (
        "plasma.colors.cppm",
        "pbsd.kde.plasma.colors",
        "pbsd::kde::plasma::colors",
        "pbsd/theme/plasma/color-schemes/PBSDAero.colors",
        "PBSDAero accent palette (brand blue, no purple slop).",
        [
            "enum class AccentRole : unsigned char {",
            "    Primary, PrimaryLight, PrimaryDark, Highlight, Hover, FocusRing, ShutdownHover",
            "};",
            "struct Rgb { unsigned char r; unsigned char g; unsigned char b; };",
            "inline constexpr Rgb kPrimary{0x50, 0x82, 0xc8};",
            "inline constexpr Rgb kPrimaryLight{0x78, 0xa8, 0xe8};",
            "inline constexpr Rgb kPrimaryDark{0x2a, 0x50, 0x88};",
            "inline constexpr Rgb kHighlight{0xb8, 0xd8, 0xff};",
            "inline constexpr Rgb kHover{0x78, 0xa8, 0xe8};",
            "inline constexpr Rgb kFocusRing{0x50, 0x82, 0xc8};",
            "inline constexpr Rgb kShutdownHover{0xe8, 0x78, 0x30};",
            'inline constexpr const char kColorSchemeName[] = "PBSDAero";',
            'inline constexpr const char kColorSchemePath[] = "plasma/color-schemes/PBSDAero.colors";',
            "[[nodiscard]] inline Rgb accent(AccentRole role) noexcept {",
            "    switch (role) {",
            "    case AccentRole::Primary: return kPrimary;",
            "    case AccentRole::PrimaryLight: return kPrimaryLight;",
            "    case AccentRole::PrimaryDark: return kPrimaryDark;",
            "    case AccentRole::Highlight: return kHighlight;",
            "    case AccentRole::Hover: return kHover;",
            "    case AccentRole::FocusRing: return kFocusRing;",
            "    case AccentRole::ShutdownHover: return kShutdownHover;",
            "    }",
            "    return kPrimary;",
            "}",
        ],
        ["import pbsd.kde.plasma.aero;"],
    ),
    (
        "plasma.systemlinks.cppm",
        "pbsd.kde.plasma.systemlinks",
        "pbsd::kde::plasma::systemlinks",
        "pbsd/theme/plasma/panel/layout.json",
        "Start menu system link tiles (layout.json systemLinks).",
        [
            "enum class Link : unsigned char {",
            "    Documents, Pictures, ControlPanel, Music, Videos, Games, Computer, Network, Devices",
            "};",
            "inline constexpr unsigned kCount{9};",
            'inline constexpr const char kDocumentsSvg[] = "plasma/panel/documents.svg";',
            'inline constexpr const char kPicturesSvg[] = "plasma/panel/pictures.svg";',
            'inline constexpr const char kControlPanelSvg[] = "plasma/panel/control-panel.svg";',
            'inline constexpr const char kMusicSvg[] = "plasma/panel/music.svg";',
            'inline constexpr const char kVideosSvg[] = "plasma/panel/videos.svg";',
            'inline constexpr const char kGamesSvg[] = "plasma/panel/games.svg";',
            'inline constexpr const char kComputerSvg[] = "plasma/panel/computer.svg";',
            'inline constexpr const char kNetworkSvg[] = "plasma/panel/network.svg";',
            'inline constexpr const char kDevicesSvg[] = "plasma/panel/devices.svg";',
            "inline constexpr float kGlassOpacity{0.68f};",
        ],
        ["import pbsd.kde.plasma.startmenu;"],
    ),
    (
        "plasma.poweractions.cppm",
        "pbsd.kde.plasma.poweractions",
        "pbsd::kde::plasma::poweractions",
        "pbsd/theme/plasma/panel/layout.json",
        "Start menu power action buttons (layout.json powerActions).",
        [
            "enum class Action : unsigned char { Lock, Sleep, Restart, Logoff, Shutdown, PowerButton };",
            "inline constexpr unsigned kCount{6};",
            'inline constexpr const char kLockSvg[] = "plasma/panel/lock.svg";',
            'inline constexpr const char kSleepSvg[] = "plasma/panel/sleep.svg";',
            'inline constexpr const char kRestartSvg[] = "plasma/panel/restart.svg";',
            'inline constexpr const char kLogoffSvg[] = "plasma/panel/logoff.svg";',
            'inline constexpr const char kShutdownSvg[] = "plasma/panel/shutdown.svg";',
            'inline constexpr const char kPowerButtonSvg[] = "plasma/panel/power-button.svg";',
            "enum class Severity : unsigned char { Normal, Caution, Destructive };",
            "inline constexpr Severity kShutdownSeverity{Severity::Destructive};",
        ],
        None,
    ),
    (
        "plasma.quickaccess.cppm",
        "pbsd.kde.plasma.quickaccess",
        "pbsd::kde::plasma::quickaccess",
        "pbsd/theme/plasma/panel/layout.json",
        "Start menu quick access links (layout.json quickAccess).",
        [
            "enum class Entry : unsigned char {",
            "    Favorites, Recent, AllPrograms, Help, Settings, Run, Search",
            "};",
            "inline constexpr unsigned kCount{7};",
            'inline constexpr const char kFavoritesSvg[] = "plasma/panel/favorites.svg";',
            'inline constexpr const char kRecentSvg[] = "plasma/panel/recent.svg";',
            'inline constexpr const char kAllProgramsSvg[] = "plasma/panel/all-programs.svg";',
            'inline constexpr const char kHelpSvg[] = "plasma/panel/help.svg";',
            'inline constexpr const char kSettingsSvg[] = "plasma/panel/settings.svg";',
            'inline constexpr const char kRunSvg[] = "plasma/panel/run.svg";',
            'inline constexpr const char kSearchSvg[] = "plasma/panel/search.svg";',
        ],
        None,
    ),
]

KWIN: list[tuple[str, str, str, str, str, list[str], list[str] | None]] = [
    (
        "kwin/aero.glow.cppm",
        "pbsd.kde.kwin.aero.glow",
        "pbsd::kde::kwin::aero::glow",
        "pbsd/theme/plasma/blur-effect.json",
        "Aero window glow blur constants (glowActive/glowInactive assets).",
        [
            'inline constexpr const char kGlowActiveSvg[] = "plasma/decoration/glow-active.svg";',
            'inline constexpr const char kGlowInactiveSvg[] = "plasma/decoration/glow-inactive.svg";',
            "inline constexpr int kGlowBlurRadius{20};",
            "inline constexpr float kGlowSaturation{1.20f};",
            "inline constexpr float kGlowContrast{1.08f};",
            "inline constexpr float kGlowNoise{0.03f};",
            "enum class State : unsigned char { Active, Inactive };",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams params(State state) noexcept {",
            "    auto p = ::pbsd::kde::kwin::compositing::glow_blur();",
            "    if (state == State::Inactive) {",
            "        p.saturation = 1.05f;",
            "        p.panel_opacity = 0.60f;",
            "    }",
            "    return p;",
            "}",
        ],
        ["import pbsd.kde.plasma.aero;", "import pbsd.kde.kwin.compositing;"],
    ),
    (
        "kwin/aero.panel.cppm",
        "pbsd.kde.kwin.aero.panel",
        "pbsd::kde::kwin::aero::panel",
        "pbsd/theme/plasma/panel/layout.json",
        "KWin panel glass compositing bridge (panel opacity + blur).",
        [
            "inline constexpr int kPanelHeight{40};",
            "inline constexpr float kPanelOpacity{0.72f};",
            "inline constexpr int kBlurRadius{24};",
            'inline constexpr const char kPanelGlassSvg[] = "plasma/panel/panel-glass.svg";',
            "enum class Edge : unsigned char { Top, Bottom, Left, Right };",
            "inline constexpr Edge kDefaultEdge{Edge::Bottom};",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur() noexcept {",
            "    return ::pbsd::kde::kwin::compositing::panel_blur();",
            "}",
        ],
        ["import pbsd.kde.plasma.aero;", "import pbsd.kde.kwin.compositing;"],
    ),
]

THEME_MODULES: list[tuple[str, str, str, list[str], list[str]]] = [
    (
        "aero.colors.cppm",
        "pbsd.theme.plasma.aero.colors",
        "pbsd::theme::plasma::aero::colors",
        ["import pbsd.kde.plasma.colors;", "import pbsd.kde.plasma.aero;"],
        [
            "using AccentRole = ::pbsd::kde::plasma::colors::AccentRole;",
            "using Rgb = ::pbsd::kde::plasma::colors::Rgb;",
            "inline constexpr Rgb kGlassTint{0x1e, 0x38, 0x66};",
            "inline constexpr float kGlassAlpha{0.65f};",
            'inline constexpr const char kSchemeName[] = "PBSDAero";',
            "[[nodiscard]] inline Rgb accent(AccentRole role) noexcept {",
            "    return ::pbsd::kde::plasma::colors::accent(role);",
            "}",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::GlassTint glass_tint() noexcept {",
            "    return ::pbsd::kde::plasma::aero::default_blur().tint;",
            "}",
        ],
    ),
    (
        "aero.dialogs.cppm",
        "pbsd.theme.plasma.aero.dialogs",
        "pbsd::theme::plasma::aero::dialogs",
        ["import pbsd.kde.plasma.dialogs;", "import pbsd.kde.plasma.aero;"],
        [
            "using Kind = ::pbsd::kde::plasma::dialogs::Kind;",
            "inline constexpr float kOpacity{0.78f};",
            "inline constexpr int kCornerRadius{8};",
            "inline constexpr int kTitleBarHeight{36};",
            "enum class Backdrop : unsigned char { Glass, Solid, Mica };",
            "inline constexpr Backdrop kDefaultBackdrop{Backdrop::Glass};",
        ],
    ),
    (
        "aero.widgets.cppm",
        "pbsd.theme.plasma.aero.widgets",
        "pbsd::theme::plasma::aero::widgets",
        ["import pbsd.kde.plasma.widgets;"],
        [
            "using Asset = ::pbsd::kde::plasma::widgets::Asset;",
            "inline constexpr int kIconSize{22};",
            "inline constexpr int kFrameRadius{4};",
            "inline constexpr float kHoverOpacity{0.85f};",
            "inline constexpr float kPressedOpacity{0.70f};",
        ],
    ),
    (
        "aero.panel.cppm",
        "pbsd.theme.plasma.aero.panel",
        "pbsd::theme::plasma::aero::panel",
        ["import pbsd.kde.plasma.panel;", "import pbsd.kde.kwin.aero.panel;"],
        [
            "inline constexpr int kHeight{40};",
            "inline constexpr float kOpacity{0.72f};",
            "inline constexpr int kBlurRadius{24};",
            'inline constexpr const char kGlassSvg[] = "plasma/panel/panel-glass.svg";',
            "enum class Position : unsigned char { Top, Bottom, Left, Right };",
            "inline constexpr Position kDefaultPosition{Position::Bottom};",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur() noexcept {",
            "    return ::pbsd::kde::kwin::aero::panel::blur();",
            "}",
        ],
    ),
    (
        "aero.glow.cppm",
        "pbsd.theme.plasma.aero.glow",
        "pbsd::theme::plasma::aero::glow",
        ["import pbsd.kde.kwin.aero.glow;"],
        [
            "using State = ::pbsd::kde::kwin::aero::glow::State;",
            "inline constexpr int kBlurRadius{20};",
            "inline constexpr float kSaturation{1.20f};",
            'inline constexpr const char kActiveSvg[] = "plasma/decoration/glow-active.svg";',
            'inline constexpr const char kInactiveSvg[] = "plasma/decoration/glow-inactive.svg";',
        ],
    ),
    (
        "aero.system_links.cppm",
        "pbsd.theme.plasma.aero.system_links",
        "pbsd::theme::plasma::aero::system_links",
        ["import pbsd.kde.plasma.systemlinks;"],
        [
            "using Link = ::pbsd::kde::plasma::systemlinks::Link;",
            "inline constexpr unsigned kCount{::pbsd::kde::plasma::systemlinks::kCount};",
            "inline constexpr float kTileOpacity{0.68f};",
            "inline constexpr int kTileSize{48};",
        ],
    ),
    (
        "aero.power_actions.cppm",
        "pbsd.theme.plasma.aero.power_actions",
        "pbsd::theme::plasma::aero::power_actions",
        ["import pbsd.kde.plasma.poweractions;"],
        [
            "using Action = ::pbsd::kde::plasma::poweractions::Action;",
            "using Severity = ::pbsd::kde::plasma::poweractions::Severity;",
            "inline constexpr unsigned kCount{::pbsd::kde::plasma::poweractions::kCount};",
        ],
    ),
    (
        "aero.notifications.cppm",
        "pbsd.theme.plasma.aero.notifications",
        "pbsd::theme::plasma::aero::notifications",
        ["import pbsd.kde.plasma.notifications;", "import pbsd.kde.plasma.aero;"],
        [
            "inline constexpr float kGlassOpacity{0.78f};",
            "inline constexpr unsigned kMaxVisible{5};",
            "inline constexpr unsigned kTimeoutMs{5000};",
            'inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";',
            "enum class Placement : unsigned char { TopRight, BottomRight, TopLeft, BottomLeft };",
            "inline constexpr Placement kDefaultPlacement{Placement::BottomRight};",
        ],
    ),
    (
        "aero.compositing.cppm",
        "pbsd.theme.plasma.aero.compositing",
        "pbsd::theme::plasma::aero::compositing",
        ["import pbsd.kde.kwin.compositing;", "import pbsd.kde.plasma.aero;"],
        [
            "enum class Surface : unsigned char { Panel, Dialog, StartMenu, Decoration, Glow, SystemLink };",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur(Surface s) noexcept {",
            "    using namespace ::pbsd::kde::kwin::compositing;",
            "    switch (s) {",
            "    case Surface::Panel: return panel_blur();",
            "    case Surface::Dialog: return dialog_blur();",
            "    case Surface::StartMenu: return start_menu_blur();",
            "    case Surface::Decoration: return decoration_blur();",
            "    case Surface::Glow: return glow_blur();",
            "    case Surface::SystemLink: return system_link_blur();",
            "    }",
            "    return ::pbsd::kde::plasma::aero::default_blur();",
            "}",
        ],
    ),
    (
        "aero.accent.cppm",
        "pbsd.theme.plasma.aero.accent",
        "pbsd::theme::plasma::aero::accent",
        ["import pbsd.kde.plasma.colors;"],
        [
            "/// Windows 7 Aero accent strip — PBSD brand blue only.",
            "inline constexpr unsigned char kPrimaryR{0x50};",
            "inline constexpr unsigned char kPrimaryG{0x82};",
            "inline constexpr unsigned char kPrimaryB{0xc8};",
            "inline constexpr unsigned char kHighlightR{0xb8};",
            "inline constexpr unsigned char kHighlightG{0xd8};",
            "inline constexpr unsigned char kHighlightB{0xff};",
            "enum class Strip : unsigned char { TitleBar, StartOrb, TaskPreview, Selection };",
        ],
    ),
]


def extract_list(content: str, var: str) -> list[str]:
    m = re.search(rf"set\({var}\s*\n(.*?)\)", content, re.S)
    if not m:
        raise SystemExit(f"missing {var}")
    return re.findall(r"^\s+([\w./-]+\.cppm)\s*$", m.group(1), re.M)


def replace_list(content: str, var: str, items: list[str]) -> str:
    block = "set(" + var + "\n" + "".join(f"    {i}\n" for i in items) + ")"
    return re.sub(rf"set\({var}\s*\n.*?\)", block, content, count=1, flags=re.S)


def write_modules() -> tuple[list[str], list[str]]:
    kde_created: list[str] = []
    for rel, mod, ns, upstream, doc, body, extra in PLASMA + KWIN:
        path = KDE / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(simple_module(mod, ns, upstream, doc, body, extra), encoding="utf-8")
        kde_created.append(rel)

    theme_created: list[str] = []
    THEME.mkdir(parents=True, exist_ok=True)
    for rel, mod, ns, imports, body in THEME_MODULES:
        path = THEME / rel
        path.write_text(theme_module(rel, mod, ns, imports, body), encoding="utf-8")
        theme_created.append(f"plasma/{rel}")
    return kde_created, theme_created


def patch_cmake(kde_new: list[str], theme_new: list[str]) -> None:
    kde_content = CMAKE_KDE.read_text(encoding="utf-8")
    plasma = extract_list(kde_content, "PBSD_KDE_PLASMA_MODULES")
    kwin = extract_list(kde_content, "PBSD_KDE_KWIN_MODULES")
    for rel in kde_new:
        if rel.startswith("plasma.") and rel not in plasma:
            plasma.append(rel)
        elif rel.startswith("kwin/") and rel not in kwin:
            kwin.append(rel)
    kde_content = replace_list(kde_content, "PBSD_KDE_PLASMA_MODULES", plasma)
    kde_content = replace_list(kde_content, "PBSD_KDE_KWIN_MODULES", kwin)
    CMAKE_KDE.write_text(kde_content, encoding="utf-8")

    theme_content = CMAKE_THEME.read_text(encoding="utf-8")
    theme_mods = extract_list(theme_content, "PBSD_THEME_PLASMA_AERO_MODULES")
    for rel in theme_new:
        if rel not in theme_mods:
            theme_mods.append(rel)
    theme_content = replace_list(theme_content, "PBSD_THEME_PLASMA_AERO_MODULES", theme_mods)
    CMAKE_THEME.write_text(theme_content, encoding="utf-8")


def bump_theme_version() -> None:
    aero = KDE / "plasma.aero.cppm"
    text = aero.read_text(encoding="utf-8")
    text = text.replace('kThemeVersion[] = "0.7.0"', 'kThemeVersion[] = "0.8.0"')
    aero.write_text(text, encoding="utf-8")

    for path in [
        THEME / "theme-colors.json",
        THEME / "blur-effect.json",
        THEME / "metadata.json",
        THEME / "decoration" / "config.json",
        THEME / "decoration" / "metadata.json",
    ]:
        if path.exists():
            t = path.read_text(encoding="utf-8")
            t = t.replace("0.7.0", "0.8.0")
            path.write_text(t, encoding="utf-8")


def main() -> None:
    kde, theme = write_modules()
    patch_cmake(kde, theme)
    bump_theme_version()
    print(f"Pass 6: {len(kde)} KDE modules, {len(theme)} theme modules")
    for c in sorted(kde + theme):
        print(c)


if __name__ == "__main__":
    main()
