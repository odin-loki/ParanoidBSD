#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 3 pass-5 KDE + Aero theme C++23 modules."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
KDE = ROOT / "pbsd" / "kde"
THEME = ROOT / "pbsd" / "theme" / "plasma"

ModuleSpec = tuple[str, str, str, str, str, list[str], list[str] | None]


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

/// Wave 3 pass 5 — {doc}
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


def effect_module(
    mod: str,
    ns: str,
    upstream: str,
    doc: str,
    cat_upper: str,
    cat_name: str,
    effect_id: str,
    display_name: str,
    category: str,
) -> str:
    return f"""export module {mod};

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 5 — {doc}
/// Upstream: {upstream}
export namespace {ns} {{

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{{
    "{cat_upper}",
    "{cat_name}",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "{upstream}",
}};

inline constexpr const char kEffectId[] = "{effect_id}";
inline constexpr const char kDisplayName[] = "{display_name}";
inline constexpr const char kPluginCategory[] = "{category}";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


FRAMEWORKS: list[tuple[str, str, str, str, str, list[str], list[str] | None]] = [
    (
        "frameworks/kwidgets.messagewidget.cppm",
        "pbsd.kde.frameworks.kwidgets.messagewidget",
        "pbsd::kde::frameworks::kwidgets::messagewidget",
        "kde/frameworks/kwidgetsaddons/src/kmessagewidget.cpp",
        "KMessageWidget message type and layout constants.",
        [
            "enum class MessageType : unsigned char { Positive, Information, Warning, Error };",
            "inline constexpr unsigned kDefaultTimeoutMs = 5000;",
            "inline constexpr int kDefaultHeight = 32;",
            "inline constexpr int kIconSize = 22;",
        ],
        None,
    ),
    (
        "frameworks/kwidgets.kswitch.cppm",
        "pbsd.kde.frameworks.kwidgets.kswitch",
        "pbsd::kde::frameworks::kwidgets::kswitch",
        "kde/frameworks/kwidgetsaddons/src/kswitch.cpp",
        "KSwitch toggle widget constants.",
        [
            "inline constexpr int kDefaultWidth = 48;",
            "inline constexpr int kDefaultHeight = 24;",
            "inline constexpr float kAnimationDurationMs = 150.0f;",
        ],
        None,
    ),
    (
        "frameworks/kwidgets.lineedit.cppm",
        "pbsd.kde.frameworks.kwidgets.lineedit",
        "pbsd::kde::frameworks::kwidgets::lineedit",
        "kde/frameworks/kwidgetsaddons/src/klineedit.cpp",
        "KLineEdit clear-button and frame constants.",
        [
            "inline constexpr int kClearButtonSize = 16;",
            "inline constexpr int kFrameMargin = 4;",
            "inline constexpr unsigned kMaxLengthDefault = 32767;",
        ],
        None,
    ),
    (
        "frameworks/kwidgets.kseparator.cppm",
        "pbsd.kde.frameworks.kwidgets.kseparator",
        "pbsd::kde::frameworks::kwidgets::kseparator",
        "kde/frameworks/kwidgetsaddons/src/kseparator.cpp",
        "KSeparator orientation constants.",
        [
            "enum class Orientation : unsigned char { Horizontal, Vertical };",
            "inline constexpr int kDefaultLineWidth = 1;",
            "inline constexpr int kDefaultMargin = 4;",
        ],
        None,
    ),
    (
        "frameworks/kio.protocols.cppm",
        "pbsd.kde.frameworks.kio.protocols",
        "pbsd::kde::frameworks::kio::protocols",
        "kde/frameworks/kio/src/core/kprotocolmanager.cpp",
        "KIO protocol scheme identifiers.",
        [
            'inline constexpr const char kFile[] = "file";',
            'inline constexpr const char kHttp[] = "http";',
            'inline constexpr const char kHttps[] = "https";',
            'inline constexpr const char kFtp[] = "ftp";',
            'inline constexpr const char kTrash[] = "trash";',
            'inline constexpr const char kDesktop[] = "desktop";',
        ],
        None,
    ),
    (
        "frameworks/kio.copyjob.cppm",
        "pbsd.kde.frameworks.kio.copyjob",
        "pbsd::kde::frameworks::kio::copyjob",
        "kde/frameworks/kio/src/core/copyjob.cpp",
        "KIO copy job operation flags.",
        [
            "enum class Operation : unsigned { Copy = 0x01, Move = 0x02, Link = 0x04 };",
            "inline constexpr unsigned kDefaultPermissions = 0644;",
            "inline constexpr unsigned kMaxConcurrentFiles = 16;",
        ],
        None,
    ),
    (
        "frameworks/kio.statjob.cppm",
        "pbsd.kde.frameworks.kio.statjob",
        "pbsd::kde::frameworks::kio::statjob",
        "kde/frameworks/kio/src/core/statjob.cpp",
        "KIO stat job detail level flags.",
        [
            "enum class Details : unsigned { None = 0, Access = 0x01, Size = 0x02, User = 0x04, Group = 0x08, Permissions = 0x10, All = 0x1f };",
            "inline constexpr unsigned kDefaultTimeoutMs = 30000;",
        ],
        None,
    ),
    (
        "frameworks/kconfig.inifile.cppm",
        "pbsd.kde.frameworks.kconfig.inifile",
        "pbsd::kde::frameworks::kconfig::inifile",
        "kde/frameworks/kconfig/src/kconfigini.cpp",
        "KConfig INI file group/key limits.",
        [
            "inline constexpr unsigned kMaxGroupLen = 256;",
            "inline constexpr unsigned kMaxKeyLen = 256;",
            "inline constexpr unsigned kMaxValueLen = 4096;",
            'inline constexpr const char kDefaultGroup[] = "General";',
        ],
        None,
    ),
    (
        "frameworks/kcoreaddons.kdatetime.cppm",
        "pbsd.kde.frameworks.kcoreaddons.kdatetime",
        "pbsd::kde::frameworks::kcoreaddons::kdatetime",
        "kde/frameworks/kcoreaddons/src/lib/datetime/kdatetime.cpp",
        "KDateTime ISO format constants.",
        [
            'inline constexpr const char kIsoDateFormat[] = "yyyy-MM-dd";',
            'inline constexpr const char kIsoTimeFormat[] = "HH:mm:ss";',
            'inline constexpr const char kIsoDateTimeFormat[] = "yyyy-MM-ddTHH:mm:ss";',
        ],
        None,
    ),
    (
        "frameworks/kcoreaddons.klocalizedstring.cppm",
        "pbsd.kde.frameworks.kcoreaddons.klocalizedstring",
        "pbsd::kde::frameworks::kcoreaddons::klocalizedstring",
        "kde/frameworks/kcoreaddons/src/lib/text/klocalizedstring.cpp",
        "KLocalizedString domain and markup constants.",
        [
            'inline constexpr const char kDefaultDomain[] = "kde";',
            'inline constexpr const char kPluralSuffix[] = "_plural";',
            "inline constexpr unsigned kMaxContextLen = 128;",
        ],
        None,
    ),
    (
        "frameworks/kwindowsystem.netwm.cppm",
        "pbsd.kde.frameworks.kwindowsystem.netwm",
        "pbsd::kde::frameworks::kwindowsystem::netwm",
        "kde/frameworks/kwindowsystem/src/netwm_def.h",
        "NetWM window state bit flags.",
        [
            "enum class State : unsigned {",
            "    Modal = 1u << 0,",
            "    Sticky = 1u << 1,",
            "    MaxVert = 1u << 2,",
            "    MaxHoriz = 1u << 3,",
            "    Max = MaxVert | MaxHoriz,",
            "    Shaded = 1u << 4,",
            "    SkipTaskbar = 1u << 5,",
            "    KeepAbove = 1u << 6,",
            "    KeepBelow = 1u << 7,",
            "    Fullscreen = 1u << 11,",
            "};",
        ],
        None,
    ),
    (
        "frameworks/kwindowsystem.startupinfo.cppm",
        "pbsd.kde.frameworks.kwindowsystem.startupinfo",
        "pbsd::kde::frameworks::kwindowsystem::startupinfo",
        "kde/frameworks/kwindowsystem/src/kstartupinfo.h",
        "KStartupInfo tri-state and property keys.",
        [
            "enum class TriState : unsigned char { Yes, No, Unknown };",
            'inline constexpr const char kStartupId[] = "StartupId";',
            'inline constexpr const char kWMClass[] = "WMClass";',
            'inline constexpr const char kDesktopFile[] = "DesktopFile";',
        ],
        None,
    ),
]

PLASMA: list[tuple[str, str, str, str, str, list[str], list[str] | None]] = [
    (
        "plasma.panel.cppm",
        "pbsd.kde.plasma.panel",
        "pbsd::kde::plasma::panel",
        "kde/plasma-desktop/containments/panel/Panel.qml",
        "Plasma panel layout constants (layout.json).",
        [
            'inline constexpr int kDefaultHeight{40};',
            'inline constexpr const char kLayoutPath[] = "plasma/panel/layout.json";',
            'inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";',
            'inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";',
            'inline constexpr const char kSystemTraySvg[] = "plasma/panel/system-tray.svg";',
            'inline constexpr const char kSeparatorSvg[] = "plasma/panel/separator.svg";',
            'inline constexpr float kDefaultOpacity{0.72f};',
            'inline constexpr int kDefaultBlurRadius{24};',
            'enum class Position : unsigned char { Top, Bottom, Left, Right };',
            'inline constexpr Position kDefaultPosition{Position::Bottom};',
        ],
        ["import pbsd.kde.plasma.aero;"],
    ),
    (
        "plasma.systray.cppm",
        "pbsd.kde.plasma.systray",
        "pbsd::kde::plasma::systray",
        "kde/plasma-desktop/applets/systemtray/package/contents/config/main.xml",
        "System tray applet constants.",
        [
            'inline constexpr const char kAppletId[] = "org.kde.plasma.systemtray";',
            'inline constexpr const char kTraySvg[] = "plasma/panel/system-tray.svg";',
            "inline constexpr unsigned kMaxItems{32};",
            "inline constexpr int kIconSize{22};",
        ],
        None,
    ),
    (
        "plasma.shell.cppm",
        "pbsd.kde.plasma.shell",
        "pbsd::kde::plasma::shell",
        "kde/plasma-workspace/shell/shell.cpp",
        "Plasma workspace shell service ids.",
        [
            'inline constexpr const char kShellDBusService[] = "org.kde.plasmashell";',
            'inline constexpr const char kShellObjectPath[] = "/PlasmaShell";',
            'inline constexpr const char kDesktopContainment[] = "Desktop";',
            'inline constexpr const char kPanelContainment[] = "Panel";',
        ],
        None,
    ),
    (
        "plasma.notifications.cppm",
        "pbsd.kde.plasma.notifications",
        "pbsd::kde::plasma::notifications",
        "kde/plasma-workspace/libnotificationmanager/notificationsmodel.cpp",
        "Plasma notification manager limits.",
        [
            'inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";',
            "inline constexpr unsigned kMaxVisible{5};",
            "inline constexpr unsigned kDefaultTimeoutMs{5000};",
            "enum class Urgency : unsigned char { Low, Normal, Critical };",
        ],
        None,
    ),
    (
        "plasma.quicklaunch.cppm",
        "pbsd.kde.plasma.quicklaunch",
        "pbsd::kde::plasma::quicklaunch",
        "kde/plasma-desktop/containments/panel/plugins/quicklaunch/quicklaunch.cpp",
        "Quick launch strip constants.",
        [
            'inline constexpr const char kQuickLaunchSvg[] = "plasma/panel/quick-launch.svg";',
            "inline constexpr unsigned kMaxIcons{10};",
            "inline constexpr int kIconSize{32};",
        ],
        None,
    ),
    (
        "plasma.taskbar.cppm",
        "pbsd.kde.plasma.taskbar",
        "pbsd::kde::plasma::taskbar",
        "kde/plasma-desktop/containments/panel/Panel.qml",
        "Taskbar strip metrics (layout.json + taskmanager).",
        [
            'inline constexpr int kDefaultHeight{40};',
            'inline constexpr int kTaskIconSize{::pbsd::kde::plasma::taskmanager::kDefaultIconSize};',
            'inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";',
            'inline constexpr const char kTasksWidgetSvg[] = "plasma/widgets/tasks.svg";',
            "inline constexpr float kGlassOpacity{0.72f};",
        ],
        ["import pbsd.kde.plasma.aero;", "import pbsd.kde.plasma.taskmanager;"],
    ),
]

KWIN_EFFECTS = [
    (
        "kwin/effects/blur.cppm",
        "pbsd.kde.kwin.effects.blur",
        "pbsd::kde::kwin::effects::blur",
        "kde/kwin/src/plugins/blur/main.cpp",
        "KWin blur compositing effect factory.",
        "KWIN_BLUR",
        "kwin_blur",
        "blur",
        "Blur",
        "Appearance",
    ),
    (
        "kwin/effects/screencast.cppm",
        "pbsd.kde.kwin.effects.screencast",
        "pbsd::kde::kwin::effects::screencast",
        "kde/kwin/src/plugins/screencast/main.cpp",
        "KWin screencast effect factory.",
        "KWIN_SCREENCAST",
        "kwin_screencast",
        "screencast",
        "Screencast",
        "Tools",
    ),
    (
        "kwin/effects/qpa.cppm",
        "pbsd.kde.kwin.effects.qpa",
        "pbsd::kde::kwin::effects::qpa",
        "kde/kwin/src/plugins/qpa/main.cpp",
        "KWin QPA platform integration stub.",
        "KWIN_QPA",
        "kwin_qpa",
        "qpa",
        "QPA",
        "System",
    ),
]

THEME_MODULES: list[tuple[str, str, str, list[str], list[str]]] = [
    (
        "aero.glass.cppm",
        "pbsd.theme.plasma.aero.glass",
        "pbsd::theme::plasma::aero::glass",
        ["import pbsd.kde.plasma.aero;"],
        [
            "/// Aero glass material tints (PBSD brand blue, no purple).",
            "struct Material {",
            "    float r{0.12f};",
            "    float g{0.22f};",
            "    float b{0.40f};",
            "    float a{0.65f};",
            "};",
            "inline constexpr Material kPanelGlass{0.10f, 0.20f, 0.38f, 0.72f};",
            "inline constexpr Material kDialogGlass{0.12f, 0.22f, 0.40f, 0.78f};",
            "inline constexpr Material kStartMenuGlass{0.11f, 0.21f, 0.39f, 0.78f};",
            "inline constexpr Material kTitleBarActive{0.14f, 0.26f, 0.46f, 0.85f};",
            "inline constexpr Material kTitleBarInactive{0.10f, 0.18f, 0.32f, 0.70f};",
            "[[nodiscard]] inline Material panel() noexcept { return kPanelGlass; }",
            "[[nodiscard]] inline Material from_blur_params(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {",
            "    return Material{p.tint.r, p.tint.g, p.tint.b, p.panel_opacity};",
            "}",
        ],
    ),
    (
        "aero.dwm_blur.cppm",
        "pbsd.theme.plasma.aero.dwm_blur",
        "pbsd::theme::plasma::aero::dwm_blur",
        ["import pbsd.kde.plasma.aero;"],
        [
            "/// DWM-style blur constants (Windows 7 Aero reference).",
            "inline constexpr int kBlurRadius{24};",
            "inline constexpr float kNoiseStrength{0.04f};",
            "inline constexpr float kSaturationBoost{1.15f};",
            "inline constexpr float kContrastBoost{1.05f};",
            "inline constexpr float kPanelOpacity{0.72f};",
            "inline constexpr float kDialogOpacity{0.78f};",
            "inline constexpr float kStartMenuOpacity{0.78f};",
            "inline constexpr bool kBlurBehindEnabled{true};",
            "inline constexpr bool kExtendFrameIntoClientArea{true};",
            "enum class Region : unsigned char { Panel, Dialog, StartMenu, TitleBar, Thumbnail };",
            "[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams defaults() noexcept {",
            "    return ::pbsd::kde::plasma::aero::default_blur();",
            "}",
        ],
    ),
    (
        "aero.start_menu.cppm",
        "pbsd.theme.plasma.aero.start_menu",
        "pbsd::theme::plasma::aero::start_menu",
        ["import pbsd.kde.plasma.aero;", "import pbsd.kde.plasma.startmenu;"],
        [
            "inline constexpr int kWidth{480};",
            "inline constexpr int kHeight{560};",
            "inline constexpr int kSidebarWidth{240};",
            "inline constexpr int kSearchHeight{36};",
            "inline constexpr int kItemHeight{32};",
            "inline constexpr float kOpacity{0.78f};",
            'inline constexpr const char kFrameSvg[] = "plasma/panel/start-menu-frame.svg";',
            'inline constexpr const char kBackgroundSvg[] = "plasma/panel/start-menu.svg";',
            "[[nodiscard]] inline ::pbsd::kde::plasma::startmenu::Layout layout() noexcept {",
            "    return ::pbsd::kde::plasma::startmenu::default_layout();",
            "}",
        ],
    ),
    (
        "aero.taskbar.cppm",
        "pbsd.theme.plasma.aero.taskbar",
        "pbsd::theme::plasma::aero::taskbar",
        ["import pbsd.kde.plasma.aero;", "import pbsd.kde.plasma.panel;"],
        [
            "inline constexpr int kHeight{40};",
            "inline constexpr int kIconSize{32};",
            "inline constexpr float kOpacity{0.72f};",
            "inline constexpr int kBlurRadius{24};",
            'inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";',
            'inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";',
            'inline constexpr const char kShowDesktopSvg[] = "plasma/panel/show-desktop.svg";',
            'inline constexpr const char kClockSvg[] = "plasma/panel/clock.svg";',
            "enum class Alignment : unsigned char { Left, Center, Right };",
            "inline constexpr Alignment kDefaultAlignment{Alignment::Center};",
        ],
    ),
    (
        "aero.window_chrome.cppm",
        "pbsd.theme.plasma.aero.window_chrome",
        "pbsd::theme::plasma::aero::window_chrome",
        ["import pbsd.kde.plasma.aero;"],
        [
            "/// KDecoration3 / Aero window chrome metrics (decoration/config.json).",
            "struct Metrics {",
            "    int title_bar_height{36};",
            "    int button_size{28};",
            "    int border_width{1};",
            "    int corner_radius{8};",
            "    bool blur_enabled{true};",
            "    float glass_opacity{0.78f};",
            "};",
            'inline constexpr const char kDecorationId[] = "pbsd_aero";',
            'inline constexpr const char kColorScheme[] = "PBSDAero";',
            'inline constexpr const char kTitleBarActiveSvg[] = "plasma/decoration/titlebar-active.svg";',
            'inline constexpr const char kTitleBarInactiveSvg[] = "plasma/decoration/titlebar-inactive.svg";',
            'inline constexpr const char kCloseSvg[] = "plasma/decoration/close.svg";',
            'inline constexpr const char kMaximizeSvg[] = "plasma/decoration/maximize.svg";',
            'inline constexpr const char kMinimizeSvg[] = "plasma/decoration/minimize.svg";',
            "[[nodiscard]] inline Metrics defaults() noexcept {",
            "    Metrics m{};",
            "    m.title_bar_height = ::pbsd::kde::plasma::aero::default_blur().title_bar_height;",
            "    m.corner_radius = ::pbsd::kde::plasma::aero::default_blur().corner_radius;",
            "    return m;",
            "}",
        ],
    ),
]


def write_kde_modules() -> list[str]:
    created: list[str] = []
    for rel, mod, ns, upstream, doc, body, extra in FRAMEWORKS + PLASMA:
        path = KDE / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        # strip duplicate import lines accidentally embedded in body
        clean_body = [line for line in body if not line.strip().startswith("import ")]
        path.write_text(simple_module(mod, ns, upstream, doc, clean_body, extra), encoding="utf-8")
        created.append(rel)

    for rel, mod, ns, upstream, doc, cat_u, cat_n, eid, dname, cat in KWIN_EFFECTS:
        path = KDE / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(effect_module(mod, ns, upstream, doc, cat_u, cat_n, eid, dname, cat), encoding="utf-8")
        created.append(rel)
    return created


def write_theme_modules() -> list[str]:
    created: list[str] = []
    THEME.mkdir(parents=True, exist_ok=True)
    for rel, mod, ns, imports, body in THEME_MODULES:
        path = THEME / rel
        body_text = "\n".join(f"    {line}" for line in body)
        imports_text = "\n".join(imports)
        text = f"""export module {mod};

import pbsd.core;
{imports_text}

/// Wave 3 pass 5 — PBSD Aero theme constants ({rel}).
export namespace {ns} {{

{body_text}

}} // namespace {ns}
"""
        path.write_text(text, encoding="utf-8")
        created.append(f"theme/plasma/{rel}")
    return created


def main() -> None:
    kde = write_kde_modules()
    theme = write_theme_modules()
    print(f"Created {len(kde)} KDE modules, {len(theme)} theme modules")
    for c in sorted(kde + theme):
        print(c)


if __name__ == "__main__":
    main()
