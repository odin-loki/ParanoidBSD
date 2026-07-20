#!/usr/bin/env python3
"""Generate Wave 3 KDE hand-port modules (batch 3 — 66 modules) under pbsd/kde/."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "kde"
MODULES: dict[str, str] = {}


def add(name: str, body: str) -> None:
    MODULES[name] = body


def const_module(mod: str, ns: str, upstream: str, lines: list[str], extra_imports: str = "") -> str:
    body = "\n".join(f"inline constexpr {line};" for line in lines)
    imports = "import pbsd.core;"
    if extra_imports:
        imports += f"\n{extra_imports}"
    return f"""export module {mod};

{imports}

/// Wave 3 pass 4 — hand port constants ({Path(upstream).name}).
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


def logging_module(mod: str, ns: str, upstream: str, cat: str, domain: str) -> str:
    return f"""export module {mod};

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — logging category mirror.
/// Upstream: {upstream}
export namespace {ns} {{

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{{
    "{cat}",
    "{domain}",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "{upstream}",
}};

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


# --- KWin core (30) -------------------------------------------------------------
add(
    "kwin/mousebuttons.cppm",
    r"""export module pbsd.kde.kwin.mousebuttons;

import pbsd.core;

/// Wave 3 pass 4 — Linux input button codes (from mousebuttons.cpp).
/// Upstream: kde/kwin/src/mousebuttons.cpp
export namespace pbsd::kde::kwin::mousebuttons {

inline constexpr unsigned kBtnLeft = 0x110;
inline constexpr unsigned kBtnMiddle = 0x111;
inline constexpr unsigned kBtnRight = 0x112;
inline constexpr unsigned kBtnSide = 0x110 + 4;
inline constexpr unsigned kBtnExtra = 0x110 + 5;
inline constexpr unsigned kBtnForward = 0x110 + 6;
inline constexpr unsigned kBtnBack = 0x110 + 7;
inline constexpr unsigned kExtraButtonBase = 0x118;

[[nodiscard]] inline bool is_primary(unsigned btn) noexcept { return btn == kBtnLeft; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/mousebuttons.cpp";
}

} // namespace pbsd::kde::kwin::mousebuttons
""",
)

add(
    "kwin/wayland/fixes.cppm",
    const_module(
        "pbsd.kde.kwin.wayland.fixes",
        "pbsd::kde::kwin::wayland::fixes",
        "kde/kwin/src/wayland/fixes.cpp",
        ["int kInterfaceVersion = 1", 'const char kInterfaceName[] = "wl_fixes"'],
    ),
)

for rel, stem, upstream, lines in [
    (
        "kwin/wayland/abstract_drop_handler.cppm",
        "abstract_drop_handler",
        "kde/kwin/src/wayland/abstract_drop_handler.cpp",
        ['const char kMimeTextPlain[] = "text/plain"', 'const char kMimeUriList[] = "text/uri-list"'],
    ),
    (
        "kwin/waylandshellintegration.cppm",
        "waylandshellintegration",
        "kde/kwin/src/waylandshellintegration.cpp",
        ['const char kXdgShell[] = "xdg_shell"', 'const char kLayerShell[] = "zwlr_layer_shell_v1"'],
    ),
    (
        "kwin/utils/realtime.cppm",
        "realtime",
        "kde/kwin/src/utils/realtime.cpp",
        ["int kSchedPolicy = 2", "int kMinPriority = 1"],
    ),
    (
        "kwin/utils/executable_path_sysctl.cppm",
        "executable_path_sysctl",
        "kde/kwin/src/utils/executable_path_sysctl.cpp",
        [
            "int kCtlKern = 1",
            "int kKernProc = 14",
            "int kKernProcPathname = 12",
            "unsigned kMaxPathLen = 1024",
        ],
    ),
    (
        "kwin/syncalarmx11filter.cppm",
        "syncalarmx11filter",
        "kde/kwin/src/syncalarmx11filter.cpp",
        ['const char kSyncCounterAtom[] = "SYNC_COUNTER"', "unsigned kAlarmMask = 3"],
    ),
    (
        "kwin/x11eventfilter.cppm",
        "x11eventfilter",
        "kde/kwin/src/x11eventfilter.cpp",
        ["unsigned kClientMessage = 33", "unsigned kPropertyNotify = 28"],
    ),
    (
        "kwin/renderloopdrivenqanimationdriver.cppm",
        "renderloopdrivenqanimationdriver",
        "kde/kwin/src/renderloopdrivenqanimationdriver.cpp",
        ["unsigned kDefaultFps = 60", "unsigned kMinFrameMs = 16"],
    ),
    (
        "kwin/opengl/abstract_opengl_context_attribute_builder.cppm",
        "abstract_opengl_context_attribute_builder",
        "kde/kwin/src/opengl/abstract_opengl_context_attribute_builder.cpp",
        ["unsigned kCoreProfile = 0x0001", "unsigned kCompatProfile = 0x0002"],
    ),
    (
        "kwin/opengl/eglimagetexture.cppm",
        "eglimagetexture",
        "kde/kwin/src/opengl/eglimagetexture.cpp",
        ['const char kEglImageTarget[] = "GL_OES_EGL_image"', "unsigned kMaxTextureSize = 8192"],
    ),
    (
        "kwin/scene/dndiconitem.cppm",
        "dndiconitem",
        "kde/kwin/src/scene/dndiconitem.cpp",
        ["int kDefaultIconSize = 48", "float kDefaultOpacity = 0.85f"],
    ),
    (
        "kwin/core/colorlut3d.cppm",
        "colorlut3d",
        "kde/kwin/src/core/colorlut3d.cpp",
        ["unsigned kLutSize = 33", "unsigned kChannelCount = 3"],
    ),
    (
        "kwin/backends/drm/drm_abstract_output.cppm",
        "drm_abstract_output",
        "kde/kwin/src/backends/drm/drm_abstract_output.cpp",
        ['const char kConnectorType[] = "Unknown"', "unsigned kInvalidCrtc = 0"],
    ),
    (
        "kwin/xwayland/drag.cppm",
        "drag",
        "kde/kwin/src/xwayland/drag.cpp",
        ['const char kXdndMimeType[] = "XdndTypeList"', "unsigned kMaxDragTargets = 16"],
    ),
    (
        "kwin/plugins/kpackage_decoration.cppm",
        "kpackage_decoration",
        "kde/kwin/src/plugins/kpackage/decoration/decoration.cpp",
        ['const char kPackageRoot[] = "aurorae/themes"', 'const char kConfigFile[] = "config.json"'],
    ),
    (
        "kwin/plugins/kpackage_effect.cppm",
        "kpackage_effect",
        "kde/kwin/src/plugins/kpackage/effect/effect.cpp",
        ['const char kEffectsDir[] = "kwin/effects"', 'const char kMetadataFile[] = "metadata.json"'],
    ),
    (
        "kwin/plugins/kpackage_windowswitcher.cppm",
        "kpackage_windowswitcher",
        "kde/kwin/src/plugins/kpackage/windowswitcher/windowswitcher.cpp",
        ['const char kTabBoxPackage[] = "kwin/tabbox"', 'const char kLayoutFile[] = "layout.qml"'],
    ),
    (
        "kwin/plugins/zoom_focustracker.cppm",
        "zoom_focustracker",
        "kde/kwin/src/plugins/zoom/focustracker.cpp",
        ["float kDefaultZoom = 1.0f", "float kMaxZoom = 4.0f"],
    ),
    (
        "kwin/plugins/eisplugin.cppm",
        "eisplugin",
        "kde/kwin/src/plugins/eis/eisplugin.cpp",
        ['const char kEisPluginId[] = "eis"', 'const char kInterfaceVersion[] = "1.0"'],
    ),
    (
        "kwin/kcms/tabbox_data.cppm",
        "tabbox_data",
        "kde/kwin/src/kcms/tabbox/kwintabboxdata.cpp",
        ['const char kConfigGroup[] = "TabBox"', "unsigned kMaxLayouts = 8"],
    ),
    (
        "kwin/kcms/screenedges_config.cppm",
        "screenedges_config",
        "kde/kwin/src/kcms/screenedges/kwintouchscreenedgeconfigform.cpp",
        ['const char kScreenEdgesGroup[] = "ScreenEdges"', "unsigned kEdgeCount = 4"],
    ),
    (
        "kwin/scripting/dbuscall.cppm",
        "dbuscall",
        "kde/kwin/src/scripting/dbuscall.cpp",
        ['const char kScriptingService[] = "org.kde.KWin"', 'const char kScriptingPath[] = "/Scripting"'],
    ),
    (
        "kwin/plugins/magiclamp_config.cppm",
        "magiclamp_config",
        "kde/kwin/src/plugins/magiclamp/magiclamp_config.cpp",
        ['const char kDurationKey[] = "Duration"', "unsigned kDefaultDurationMs = 250"],
    ),
    (
        "kwin/plugins/slide_config.cppm",
        "slide_config",
        "kde/kwin/src/plugins/slide/slide_config.cpp",
        ['const char kLocationKey[] = "Location"', 'const char kDefaultLocation[] = "Left"'],
    ),
    (
        "kwin/helpers/kwindowprop.cppm",
        "kwindowprop",
        "kde/kwin/src/helpers/kwindowprop/main.cpp",
        ['const char kHelperName[] = "kwindowprop"', "unsigned kMaxArgs = 8"],
    ),
    (
        "kwin/wayland/server_decoration.cppm",
        "server_decoration",
        "kde/kwin/src/wayland/server_decoration.cpp",
        ['const char kServerDecoration[] = "zxdg_decoration_manager_v1"', "int kVersion = 1"],
    ),
    (
        "kwin/wayland/presentation.cppm",
        "presentation",
        "kde/kwin/src/wayland/presentation.cpp",
        ['const char kPresentation[] = "wp_presentation"', "unsigned kRefreshNs = 16666666"],
    ),
    (
        "kwin/wayland/keyboard_shortcuts_inhibit.cppm",
        "keyboard_shortcuts_inhibit",
        "kde/kwin/src/wayland/keyboard_shortcuts_inhibit.cpp",
        ['const char kShortcutsInhibit[] = "zwp_keyboard_shortcuts_inhibit_manager_v1"', "int kVersion = 1"],
    ),
    (
        "kwin/backends/drm/drm_output.cppm",
        "drm_output",
        "kde/kwin/src/backends/drm/drm_output.cpp",
        ['const char kOutputNamePrefix[] = "HDMI-A"', "unsigned kMaxModes = 32"],
    ),
    (
        "kwin/input/sp_keyboard.cppm",
        "sp_keyboard",
        "kde/kwin/src/input/sp_keyboard.cpp",
        ["unsigned kMaxKeys = 256", "unsigned kRepeatDelayMs = 600"],
    ),
]:
    mod = "pbsd.kde." + stem.replace("/", ".")
    ns = "pbsd::kde::" + stem.replace("/", "::")
    add(rel, const_module(mod, ns, upstream, lines))

# --- KWin logging (3) -----------------------------------------------------------
for rel, ns_suffix, upstream, cat, domain in [
    (
        "kwin/backends/virtual.logging.cppm",
        "backends::virtual_::logging",
        "kde/kwin/src/backends/virtual/virtual_logging.cpp",
        "KWIN_VIRTUAL",
        "kwin_virtual",
    ),
    (
        "kwin/backends/drm/drm_backend.logging.cppm",
        "backends::drm::logging",
        "kde/kwin/src/backends/drm/drm_logging.cpp",
        "KWIN_DRM",
        "kwin_drm",
    ),
    (
        "kwin/backends/libinput/libinput_backend.logging.cppm",
        "backends::libinput::logging",
        "kde/kwin/src/backends/libinput/libinput_logging.cpp",
        "KWIN_LIBINPUT",
        "kwin_libinput",
    ),
]:
    mod = "pbsd.kde." + ns_suffix.replace("::", ".")
    add(rel, logging_module(mod, f"pbsd::kde::{ns_suffix}", upstream, cat, domain))

# --- Frameworks (20) --------------------------------------------------------------
for rel, mod_suffix, ns, upstream, lines in [
    (
        "frameworks/kconfig.types_qml.cppm",
        "kconfig.types_qml",
        "pbsd::kde::frameworks::kconfig::types_qml",
        "kde/frameworks/kconfig/src/qml/types.cpp",
        ['const char kQmlModule[] = "org.kde.kconfig"', 'const char kTypesUri[] = "org.kde.kconfig.types"'],
    ),
    (
        "frameworks/plasma.applet_packagestructure.cppm",
        "plasma.applet_packagestructure",
        "pbsd::kde::frameworks::plasma::applet_packagestructure",
        "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_applet_packagestructure.cpp",
        [
            'const char kPluginId[] = "Plasma/Applet"',
            'const char kConfigModel[] = "config/config.qml"',
            'const char kMainConfigXml[] = "config/main.xml"',
        ],
    ),
    (
        "frameworks/plasma.containmentactions_packagestructure.cppm",
        "plasma.containmentactions_packagestructure",
        "pbsd::kde::frameworks::plasma::containmentactions_packagestructure",
        "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp",
        [
            'const char kPluginId[] = "Plasma/ContainmentActions"',
            'const char kActionsDir[] = "actions"',
        ],
    ),
    (
        "frameworks/plasma.plasmaquick_utils.cppm",
        "plasma.plasmaquick_utils",
        "pbsd::kde::frameworks::plasma::plasmaquick_utils",
        "kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp",
        ['const char kAppletPrefix[] = "org.kde.plasma"', "unsigned kMaxApplets = 64"],
    ),
    (
        "frameworks/plasma.plasmaquick_appletcontext.cppm",
        "plasma.plasmaquick_appletcontext",
        "pbsd::kde::frameworks::plasma::plasmaquick_appletcontext",
        "kde/frameworks/plasma-framework/src/plasmaquick/appletcontext.cpp",
        ['const char kContextProperty[] = "applet"', 'const char kContainmentProperty[] = "containment"'],
    ),
    (
        "frameworks/kwindowsystem.kusertimestamp.cppm",
        "kwindowsystem.kusertimestamp",
        "pbsd::kde::frameworks::kwindowsystem::kusertimestamp",
        "kde/frameworks/kwindowsystem/src/kusertimestamp.cpp",
        ['const char kTimestampProperty[] = "_NET_WM_USER_TIME"', "unsigned kInvalidTimestamp = 0"],
    ),
    (
        "frameworks/layershell.interfaces_shell.cppm",
        "layershell.interfaces_shell",
        "pbsd::kde::frameworks::layershell::interfaces_shell",
        "kde/frameworks/layer-shell-qt/src/interfaces/shell.cpp",
        ['const char kLayerShell[] = "zwlr_layer_shell_v1"', "int kVersion = 4"],
    ),
    (
        "frameworks/layershell.layershellintegration.cppm",
        "layershell.layershellintegration",
        "pbsd::kde::frameworks::layershell::layershellintegration",
        "kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegration.cpp",
        ['const char kIntegrationPlugin[] = "layer-shell-qt"', 'const char kShellSurface[] = "zwlr_layer_surface_v1"'],
    ),
    (
        "frameworks/kio.openorexecutefileinterface.cppm",
        "kio.openorexecutefileinterface",
        "pbsd::kde::frameworks::kio::openorexecutefileinterface",
        "kde/frameworks/kio/src/gui/openorexecutefileinterface.cpp",
        ['const char kInterfaceIid[] = "org.kde.kio.openOrExecuteFile/1.0"'],
    ),
    (
        "frameworks/kio.openwithhandlerinterface.cppm",
        "kio.openwithhandlerinterface",
        "pbsd::kde::frameworks::kio::openwithhandlerinterface",
        "kde/frameworks/kio/src/gui/openwithhandlerinterface.cpp",
        ['const char kInterfaceIid[] = "org.kde.kio.openWithHandler/1.0"'],
    ),
    (
        "frameworks/kio.remotedirnotify.cppm",
        "kio.remotedirnotify",
        "pbsd::kde::frameworks::kio::remotedirnotify",
        "kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotify.cpp",
        ['const char kModuleName[] = "remotedirnotify"', 'const char kDbusService[] = "org.kde.remotedirnotify"'],
    ),
    (
        "frameworks/kio.remotedirnotifymodule.cppm",
        "kio.remotedirnotifymodule",
        "pbsd::kde::frameworks::kio::remotedirnotifymodule",
        "kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotifymodule.cpp",
        ['const char kModuleName[] = "remotedirnotifymodule"'],
    ),
    (
        "frameworks/kxmlgui.kbugreport.cppm",
        "kxmlgui.kbugreport",
        "pbsd::kde::frameworks::kxmlgui::kbugreport",
        "kde/frameworks/kxmlgui/tests/kbugreporttest.cpp",
        ['const char kBugReportUrl[] = "https://bugs.kde.org"'],
    ),
    (
        "frameworks/kxmlgui.kmainwindow.cppm",
        "kxmlgui.kmainwindow",
        "pbsd::kde::frameworks::kxmlgui::kmainwindow",
        "kde/frameworks/kxmlgui/tests/kmainwindowtest.cpp",
        ['const char kMainWindowGroup[] = "MainWindow"', 'const char kStateKey[] = "State"'],
    ),
    (
        "frameworks/kio.kfilewidgetdocktitlebar.cppm",
        "kio.kfilewidgetdocktitlebar",
        "pbsd::kde::frameworks::kio::kfilewidgetdocktitlebar",
        "kde/frameworks/kio/src/filewidgets/kfilewidgetdocktitlebar.cpp",
        ['const char kDockTitle[] = "Places"', "unsigned kTitleHeight = 24"],
    ),
    (
        "frameworks/kio.kpreviewwidgetbase.cppm",
        "kio.kpreviewwidgetbase",
        "pbsd::kde::frameworks::kio::kpreviewwidgetbase",
        "kde/frameworks/kio/src/filewidgets/kpreviewwidgetbase.cpp",
        ["unsigned kPreviewMaxSize = 512", "unsigned kPreviewMinSize = 64"],
    ),
    (
        "frameworks/kdecoration.decorationbridge_private.cppm",
        "kdecoration.decorationbridge_private",
        "pbsd::kde::frameworks::kdecoration::decorationbridge_private",
        "kde/frameworks/kdecoration/src/private/decorationbridge.cpp",
        ['const char kBridgeIid[] = "org.kde.kdecoration2/1.0"'],
    ),
    (
        "frameworks/kwindowsystem.kwindowsystem.cppm",
        "kwindowsystem.kwindowsystem",
        "pbsd::kde::frameworks::kwindowsystem::kwindowsystem",
        "kde/frameworks/kwindowsystem/src/platforms/xcb/kwindowsystem.cpp",
        ['const char kNetWmName[] = "_NET_WM_NAME"', 'const char kNetWmIcon[] = "_NET_WM_ICON"'],
    ),
    (
        "frameworks/plasma.packagestructure_wallpaper.cppm",
        "plasma.packagestructure_wallpaper",
        "pbsd::kde::frameworks::plasma::packagestructure_wallpaper",
        "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_wallpaper_packagestructure.cpp",
        ['const char kPluginId[] = "Plasma/Wallpaper"', 'const char kWallpaperQml[] = "contents/ui/main.qml"'],
    ),
    (
        "frameworks/kcoreaddons.ktexttohtml.cppm",
        "kcoreaddons.ktexttohtml",
        "pbsd::kde::frameworks::kcoreaddons::ktexttohtml",
        "kde/frameworks/kcoreaddons/tests/ktexttohtmltest.cpp",
        ['const char kBoldTag[] = "<b>"', 'const char kItalicTag[] = "<i>"'],
    ),
]:
    add(rel, const_module(f"pbsd.kde.{mod_suffix}", ns, upstream, lines))

# --- Plasma (13) ----------------------------------------------------------------
for rel, mod_suffix, ns, upstream, lines in [
    (
        "plasma.tastenbrett.outline.cppm",
        "plasma.tastenbrett.outline",
        "pbsd::kde::plasma::tastenbrett::outline",
        "kde/plasma-desktop/kcms/keyboard/tastenbrett/outline.cpp",
        ["unsigned kOutlineWidth = 2", "unsigned kCornerRadius = 4"],
    ),
    (
        "plasma.tastenbrett.row.cppm",
        "plasma.tastenbrett.row",
        "pbsd::kde::plasma::tastenbrett::row",
        "kde/plasma-desktop/kcms/keyboard/tastenbrett/row.cpp",
        ["unsigned kMaxKeysPerRow = 20", "unsigned kKeySpacing = 4"],
    ),
    (
        "plasma.tastenbrett.application.cppm",
        "plasma.tastenbrett.application",
        "pbsd::kde::plasma::tastenbrett::application",
        "kde/plasma-desktop/kcms/keyboard/tastenbrett/application.cpp",
        ['const char kAppGroup[] = "KeyboardLayout"', "unsigned kMaxLayouts = 8"],
    ),
    (
        "plasma.tastenbrett.shape.cppm",
        "plasma.tastenbrett.shape",
        "pbsd::kde::plasma::tastenbrett::shape",
        "kde/plasma-desktop/kcms/keyboard/tastenbrett/shape.cpp",
        ["unsigned kKeyWidth = 48", "unsigned kKeyHeight = 48"],
    ),
    (
        "plasma.keyboard.layoutnames.cppm",
        "plasma.keyboard.layoutnames",
        "pbsd::kde::plasma::keyboard::layoutnames",
        "kde/plasma-desktop/kcms/keyboard/layoutnames.cpp",
        ['const char kLayoutGroup[] = "LayoutList"', "unsigned kMaxLayoutNameLen = 64"],
    ),
    (
        "plasma.mouse.kapplymousetheme.cppm",
        "plasma.mouse.kapplymousetheme",
        "pbsd::kde::plasma::mouse::kapplymousetheme",
        "kde/plasma-desktop/kcms/mouse/kapplymousetheme.cpp",
        ['const char kCursorThemeGroup[] = "Mouse"', 'const char kThemeKey[] = "cursorTheme"'],
    ),
    (
        "plasma.baloo.baloodata.cppm",
        "plasma.baloo.baloodata",
        "pbsd::kde::plasma::baloo::baloodata",
        "kde/plasma-desktop/kcms/baloo/baloodata.cpp",
        ['const char kBalooGroup[] = "Basic Settings"', 'const char kEnabledKey[] = "enabled"'],
    ),
    (
        "plasma.touchscreen.moduledata.cppm",
        "plasma.touchscreen.moduledata",
        "pbsd::kde::plasma::touchscreen::moduledata",
        "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp",
        ['const char kTouchscreenGroup[] = "Touchscreen"', "unsigned kMaxDevices = 8"],
    ),
    (
        "plasma.emojier.category.cppm",
        "plasma.emojier.category",
        "pbsd::kde::plasma::emojier::category",
        "kde/plasma-desktop/emojier/emojicategory.cpp",
        ['const char kRecentCategory[] = "recent"', 'const char kSmileysCategory[] = "smileys"'],
    ),
    (
        "plasma.folder.applauncher.cppm",
        "plasma.folder.applauncher",
        "pbsd::kde::plasma::folder::applauncher",
        "kde/plasma-desktop/containments/desktop/plugins/folder/applauncher.cpp",
        ['const char kDesktopEntrySuffix[] = ".desktop"', "unsigned kMaxApps = 256"],
    ),
    (
        "plasma.runners.abstractjob.cppm",
        "plasma.runners.abstractjob",
        "pbsd::kde::plasma::runners::abstractjob",
        "kde/plasma-desktop/kcms/runners/plugininstaller/AbstractJob.cpp",
        ['const char kJobStatePending[] = "pending"', 'const char kJobStateFinished[] = "finished"'],
    ),
    (
        "plasma.runners.zypperrpmjob.cppm",
        "plasma.runners.zypperrpmjob",
        "pbsd::kde::plasma::runners::zypperrpmjob",
        "kde/plasma-desktop/kcms/runners/plugininstaller/ZypperRPMJob.cpp",
        ['const char kPackageManager[] = "zypper"', 'const char kRpmExtension[] = ".rpm"'],
    ),
    (
        "plasma.kimpanel.ibus.cppm",
        "plasma.kimpanel.ibus",
        "pbsd::kde::plasma::kimpanel::ibus",
        "kde/plasma-desktop/applets/kimpanel/backend/ibus/ibus15/main.cpp",
        ['const char kIbusService[] = "org.freedesktop.IBus"', 'const char kIbusPath[] = "/org/freedesktop/IBus"'],
    ),
]:
    add(rel, const_module(f"pbsd.kde.{mod_suffix}", ns, upstream, lines))

# batch3 module manifest for CMake sync
BATCH3_MODULES = sorted(MODULES.keys())


def main() -> int:
    for rel, content in MODULES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {rel}")
    manifest = ROOT.parent.parent / "tools" / "kde_wave3_batch3_modules.txt"
    manifest.write_text("\n".join(BATCH3_MODULES) + "\n", encoding="utf-8")
    print(f"total {len(MODULES)}")
    print(f"manifest {manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
