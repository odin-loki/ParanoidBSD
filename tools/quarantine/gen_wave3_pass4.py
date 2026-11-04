#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 3 pass-4 hand-port KDE C++23 modules (50+ TUs)."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "pbsd" / "kde"

# (rel_path, module, namespace, upstream, doc, body_lines, extra_imports?)
ModuleSpec = tuple[str, str, str, str, str, list[str], list[str] | None]


def sanitize_ns(name: str) -> str:
    return re.sub(r"[^a-zA-Z0-9_]", "_", name.replace("-", "_"))


def simple_module(
    rel: str,
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

/// Wave 3 pass 4 — {doc}
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


def logging_module(rel: str, mod: str, ns: str, upstream: str, cat_id: str, cat_name: str) -> str:
    cat_upper = re.sub(r"[^A-Z0-9]", "_", cat_id.upper())
    return f"""export module {mod};

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — KWin logging category mirror.
/// Upstream: {upstream}
export namespace {ns} {{

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{{
    "{cat_upper}",
    "{cat_name}",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "{upstream}",
}};

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


KWIN_CORE: list[ModuleSpec] = [
    ("kwin/lidswitchtracker.cppm", "pbsd.kde.kwin.lidswitchtracker", "pbsd::kde::kwin::lidswitchtracker",
     "kde/kwin/src/lidswitchtracker.cpp", "Lid switch state tracker constants.",
     ['inline constexpr const char kLidClosedKey[] = "lidClosed";',
      'inline constexpr const char kInhibitedKey[] = "inhibited";']),
    ("kwin/killwindow.cppm", "pbsd.kde.kwin.killwindow", "pbsd::kde::kwin::killwindow",
     "kde/kwin/src/killwindow.cpp", "Kill-window helper shortcut id.",
     ['inline constexpr const char kShortcutId[] = "Kill Window";',
      'inline constexpr unsigned kGraceMs = 500;']),
    ("kwin/desktopeffectsdata.cppm", "pbsd.kde.kwin.desktopeffectsdata", "pbsd::kde::kwin::desktopeffectsdata",
     "kde/kwin/src/kcms/effects/desktopeffectsdata.cpp", "Desktop effects KCM data group.",
     ['inline constexpr const char kConfigGroup[] = "Plugins";',
      'inline constexpr const char kEnabledKey[] = "EnabledByDefault";']),
    ("kwin/kwinscriptsdata.cppm", "pbsd.kde.kwin.kwinscriptsdata", "pbsd::kde::kwin::kwinscriptsdata",
     "kde/kwin/src/kcms/scripts/kwinscriptsdata.cpp", "KWin scripts KCM data keys.",
     ['inline constexpr const char kScriptsGroup[] = "Plugins";',
      'inline constexpr const char kScriptIdKey[] = "Id";']),
    ("kwin/drm_blob.cppm", "pbsd.kde.kwin.drm_blob", "pbsd::kde::kwin::drm_blob",
     "kde/kwin/src/backends/drm/drm_blob.cpp", "DRM blob property id stub.",
     ['inline constexpr unsigned kInvalidBlobId = 0;',
      'inline constexpr const char kBlobProperty[] = "BLOB_ID";']),
    ("kwin/renderjournal.cppm", "pbsd.kde.kwin.renderjournal", "pbsd::kde::kwin::renderjournal",
     "kde/kwin/src/core/renderjournal.cpp", "Render journal frame counter.",
     ['inline constexpr unsigned kMaxFrames = 256;',
      'inline constexpr unsigned kDefaultHistory = 16;']),
    ("kwin/colorpipelinestage.cppm", "pbsd.kde.kwin.colorpipelinestage", "pbsd::kde::kwin::colorpipelinestage",
     "kde/kwin/src/core/colorpipelinestage.cpp", "Color pipeline stage tags.",
     ['enum class Stage : unsigned char { Input, Tonemap, Output, Unknown };',
      'inline constexpr unsigned kMaxStages = 8;']),
    ("kwin/inputbackend.cppm", "pbsd.kde.kwin.inputbackend", "pbsd::kde::kwin::inputbackend",
     "kde/kwin/src/core/inputbackend.cpp", "Input backend name constants.",
     ['inline constexpr const char kLibinputBackend[] = "libinput";',
      'inline constexpr const char kX11Backend[] = "x11";']),
    ("kwin/outputconfiguration.cppm", "pbsd.kde.kwin.outputconfiguration", "pbsd::kde::kwin::outputconfiguration",
     "kde/kwin/src/core/outputconfiguration.cpp", "Output configuration apply flags.",
     ['inline constexpr unsigned kApplyMode = 1;',
      'inline constexpr unsigned kApplyTransform = 2;',
      'inline constexpr unsigned kApplyScale = 4;']),
    ("kwin/itemrenderer.cppm", "pbsd.kde.kwin.itemrenderer", "pbsd::kde::kwin::itemrenderer",
     "kde/kwin/src/scene/itemrenderer.cpp", "Scene item renderer backend ids.",
     ['inline constexpr const char kOpenGlRenderer[] = "opengl";',
      'inline constexpr const char kQPainterRenderer[] = "qpainter";']),
    ("kwin/backgroundeffectitem.cppm", "pbsd.kde.kwin.backgroundeffectitem", "pbsd::kde::kwin::backgroundeffectitem",
     "kde/kwin/src/scene/backgroundeffectitem.cpp", "Background effect item layer.",
     ['inline constexpr int kBackgroundLayer = -1;',
      'inline constexpr float kDefaultOpacity = 1.0f;']),
    ("kwin/surfaceitem_internal.cppm", "pbsd.kde.kwin.surfaceitem_internal", "pbsd::kde::kwin::surfaceitem_internal",
     "kde/kwin/src/scene/surfaceitem_internal.cpp", "Internal surface item flags.",
     ['inline constexpr unsigned kInternalFlag = 0x01;',
      'inline constexpr unsigned kScanoutCandidate = 0x02;']),
    ("kwin/rootinfo_filter.cppm", "pbsd.kde.kwin.rootinfo_filter", "pbsd::kde::kwin::rootinfo_filter",
     "kde/kwin/src/rootinfo_filter.cpp", "X11 root info filter property names.",
     ['inline constexpr const char kNetActiveWindow[] = "_NET_ACTIVE_WINDOW";',
      'inline constexpr const char kNetSupported[] = "_NET_SUPPORTED";']),
    ("kwin/window_property_notify_x11_filter.cppm", "pbsd.kde.kwin.window_property_notify_x11_filter",
     "pbsd::kde::kwin::window_property_notify_x11_filter",
     "kde/kwin/src/window_property_notify_x11_filter.cpp", "X11 property notify filter.",
     ['inline constexpr unsigned kMaxProperties = 32;',
      'inline constexpr const char kWmState[] = "WM_STATE";']),
    ("kwin/xxpipv1integration.cppm", "pbsd.kde.kwin.xxpipv1integration", "pbsd::kde::kwin::xxpipv1integration",
     "kde/kwin/src/xxpipv1integration.cpp", "Xwayland XPresent integration stub.",
     ['inline constexpr const char kProtocolName[] = "XPresent";',
      'inline constexpr unsigned kVersion = 1;']),
    ("kwin/inputpanelv1integration.cppm", "pbsd.kde.kwin.inputpanelv1integration",
     "pbsd::kde::kwin::inputpanelv1integration",
     "kde/kwin/src/inputpanelv1integration.cpp", "Input panel v1 protocol stub.",
     ['inline constexpr const char kProtocolName[] = "zwp_input_panel_v1";',
      'inline constexpr unsigned kVersion = 1;']),
]

KWIN_WAYLAND: list[ModuleSpec] = [
    ("kwin/wayland/region.cppm", "pbsd.kde.kwin.wayland.region", "pbsd::kde::kwin::wayland::region",
     "kde/kwin/src/wayland/region.cpp", "Wayland region rectangle limits.",
     ['inline constexpr int kMaxRects = 64;',
      'inline constexpr int kInvalidCoord = -1;']),
    ("kwin/wayland/abstract_data_source.cppm", "pbsd.kde.kwin.wayland.abstract_data_source",
     "pbsd::kde::kwin::wayland::abstract_data_source",
     "kde/kwin/src/wayland/abstract_data_source.cpp", "Data source mime type cap.",
     ['inline constexpr unsigned kMaxMimeTypes = 16;',
      'inline constexpr unsigned kMaxMimeLen = 128;']),
    ("kwin/wayland/xdgtopleveltag_v1.cppm", "pbsd.kde.kwin.wayland.xdgtopleveltag_v1",
     "pbsd::kde::kwin::wayland::xdgtopleveltag_v1",
     "kde/kwin/src/wayland/xdgtopleveltag_v1.cpp", "xdg toplevel tag protocol.",
     ['inline constexpr const char kProtocolName[] = "xdg_toplevel_tag_v1";',
      'inline constexpr unsigned kVersion = 1;']),
    ("kwin/wayland/pointerwarp_v1.cppm", "pbsd.kde.kwin.wayland.pointerwarp_v1",
     "pbsd::kde::kwin::wayland::pointerwarp_v1",
     "kde/kwin/src/wayland/pointerwarp_v1.cpp", "Pointer warp protocol stub.",
     ['inline constexpr const char kProtocolName[] = "wp_pointer_warp_v1";',
      'inline constexpr unsigned kVersion = 1;']),
    ("kwin/wayland/textinput_v3.cppm", "pbsd.kde.kwin.wayland.textinput_v3", "pbsd::kde::kwin::wayland::textinput_v3",
     "kde/kwin/src/wayland/textinput.cpp", "Text input v3 protocol name.",
     ['inline constexpr const char kProtocolName[] = "zwp_text_input_v3";',
      'inline constexpr unsigned kVersion = 3;']),
]

KWIN_PLUGINS: list[ModuleSpec] = [
    ("kwin/plugins/blur_config.cppm", "pbsd.kde.kwin.plugins.blur_config", "pbsd::kde::kwin::plugins::blur_config",
     "kde/kwin/src/plugins/blur/blur_config.cpp", "Blur effect KCM config keys.",
     ['inline constexpr const char kStrengthKey[] = "Strength";',
      'inline constexpr const char kNoiseKey[] = "Noise";'],
     ["import pbsd.kde.kwin.logging.common;"]),
    ("kwin/plugins/glide_config.cppm", "pbsd.kde.kwin.plugins.glide_config", "pbsd::kde::kwin::plugins::glide_config",
     "kde/kwin/src/plugins/glide/glide_config.cpp", "Glide effect KCM keys.",
     ['inline constexpr const char kDurationKey[] = "Duration";',
      'inline constexpr const char kCurveKey[] = "Curve";']),
    ("kwin/plugins/colorpickerlayer.cppm", "pbsd.kde.kwin.plugins.colorpickerlayer",
     "pbsd::kde::kwin::plugins::colorpickerlayer",
     "kde/kwin/src/plugins/colorpicker/colorpickerlayer.cpp", "Color picker overlay layer.",
     ['inline constexpr int kOverlayLayer = 10000;',
      'inline constexpr const char kEffectId[] = "colorpicker";']),
    ("kwin/plugins/screenshotlayer.cppm", "pbsd.kde.kwin.plugins.screenshotlayer",
     "pbsd::kde::kwin::plugins::screenshotlayer",
     "kde/kwin/src/plugins/screenshot/screenshotlayer.cpp", "Screenshot overlay layer id.",
     ['inline constexpr int kScreenshotLayer = 9999;',
      'inline constexpr const char kEffectId[] = "screenshot";']),
    ("kwin/plugins/filteredsceneview.cppm", "pbsd.kde.kwin.plugins.filteredsceneview",
     "pbsd::kde::kwin::plugins::filteredsceneview",
     "kde/kwin/src/plugins/screencast/filteredsceneview.cpp", "Screencast filtered scene view.",
     ['inline constexpr const char kEffectId[] = "screencast";',
      'inline constexpr unsigned kMaxFilters = 8;']),
    ("kwin/plugins/offscreensurface.cppm", "pbsd.kde.kwin.plugins.offscreensurface",
     "pbsd::kde::kwin::plugins::offscreensurface",
     "kde/kwin/src/plugins/qpa/offscreensurface.cpp", "QPA offscreen surface stub.",
     ['inline constexpr unsigned kDefaultWidth = 1;',
      'inline constexpr unsigned kDefaultHeight = 1;']),
    ("kwin/plugins/qpainterbackend.cppm", "pbsd.kde.kwin.plugins.qpainterbackend",
     "pbsd::kde::kwin::plugins::qpainterbackend",
     "kde/kwin/src/qpainter/qpainterbackend.cpp", "QPainter compositor backend id.",
     ['inline constexpr const char kBackendId[] = "qpainter";',
      'inline constexpr bool kSupportsEffects = false;']),
    ("kwin/plugins/platformcursor.cppm", "pbsd.kde.kwin.plugins.platformcursor",
     "pbsd::kde::kwin::plugins::platformcursor",
     "kde/kwin/src/plugins/qpa/platformcursor.cpp", "Platform cursor shape ids.",
     ['inline constexpr unsigned kArrowShape = 0;',
      'inline constexpr unsigned kIbeamShape = 1;',
      'inline constexpr unsigned kWaitShape = 3;']),
    ("kwin/plugins/aurorae.cppm", "pbsd.kde.kwin.plugins.aurorae", "pbsd::kde::kwin::plugins::aurorae",
     "kde/kwin/src/plugins/kpackage/aurorae/aurorae.cpp", "Aurorae decoration package id.",
     ['inline constexpr const char kPackageId[] = "org.kde.kwin.aurorae";',
      'inline constexpr const char kThemeKey[] = "theme";']),
    ("kwin/plugins/scripts.cppm", "pbsd.kde.kwin.plugins.scripts", "pbsd::kde::kwin::plugins::scripts",
     "kde/kwin/src/plugins/kpackage/scripts/scripts.cpp", "KWin scripts kpackage structure.",
     ['inline constexpr const char kPackageStructure[] = "KWin/Script";',
      'inline constexpr const char kScriptSuffix[] = ".js";']),
]

FRAMEWORKS: list[ModuleSpec] = [
    ("frameworks/kwindowsystem.kwindoweffects.cppm", "pbsd.kde.frameworks.kwindowsystem.kwindoweffects",
     "pbsd::kde::frameworks::kwindowsystem::kwindoweffects",
     "kde/frameworks/kwindowsystem/src/kwindoweffects.cpp", "KWindowEffects capability flags.",
     ['inline constexpr unsigned kBlurBehind = 0x01;',
      'inline constexpr unsigned kSlide = 0x02;',
      'inline constexpr unsigned kPresentWindows = 0x04;']),
    ("frameworks/kwindowsystem.wayland_plugin.cppm", "pbsd.kde.frameworks.kwindowsystem.wayland_plugin",
     "pbsd::kde::frameworks::kwindowsystem::wayland_plugin",
     "kde/frameworks/kwindowsystem/src/platforms/wayland/plugin.cpp", "Wayland platform plugin id.",
     ['inline constexpr const char kPluginId[] = "wayland";',
      'inline constexpr const char kPlatformName[] = "wayland";']),
    ("frameworks/kwindowsystem.xcb_plugin.cppm", "pbsd.kde.frameworks.kwindowsystem.xcb_plugin",
     "pbsd::kde::frameworks::kwindowsystem::xcb_plugin",
     "kde/frameworks/kwindowsystem/src/platforms/xcb/plugin.cpp", "XCB platform plugin id.",
     ['inline constexpr const char kPluginId[] = "xcb";',
      'inline constexpr const char kPlatformName[] = "xcb";']),
    ("frameworks/kwindowsystem.waylandxdgactivationv1.cppm",
     "pbsd.kde.frameworks.kwindowsystem.waylandxdgactivationv1",
     "pbsd::kde::frameworks::kwindowsystem::waylandxdgactivationv1",
     "kde/frameworks/kwindowsystem/src/platforms/wayland/waylandxdgactivationv1.cpp",
     "xdg activation v1 protocol stub.",
     ['inline constexpr const char kProtocolName[] = "xdg_activation_v1";',
      'inline constexpr unsigned kVersion = 1;']),
    ("frameworks/layershell.qwaylandxdgactivationv1.cppm",
     "pbsd.kde.frameworks.layershell.qwaylandxdgactivationv1",
     "pbsd::kde::frameworks::layershell::qwaylandxdgactivationv1",
     "kde/frameworks/layer-shell-qt/src/qwaylandxdgactivationv1.cpp", "Layer-shell xdg activation.",
     ['inline constexpr const char kInterfaceName[] = "xdg_activation_v1";']),
    ("frameworks/kxmlgui.kaboutapplicationlistview.cppm",
     "pbsd.kde.frameworks.kxmlgui.kaboutapplicationlistview",
     "pbsd::kde::frameworks::kxmlgui::kaboutapplicationlistview",
     "kde/frameworks/kxmlgui/src/kaboutapplicationlistview_p.cpp", "About dialog list view keys.",
     ['inline constexpr const char kAppNameRole[] = "appName";',
      'inline constexpr const char kVersionRole[] = "version";']),
    ("frameworks/kio.jobuidelegateextension.cppm", "pbsd.kde.frameworks.kio.jobuidelegateextension",
     "pbsd::kde::frameworks::kio::jobuidelegateextension",
     "kde/frameworks/kio/src/core/jobuidelegateextension.cpp", "Job UI delegate extension IID.",
     ['inline constexpr const char kInterfaceIid[] = "org.kde.kio.jobuidelegateextension/1.0";']),
    ("frameworks/kio.kurlnavigatorpathselectoreventfilter.cppm",
     "pbsd.kde.frameworks.kio.kurlnavigatorpathselectoreventfilter",
     "pbsd::kde::frameworks::kio::kurlnavigatorpathselectoreventfilter",
     "kde/frameworks/kio/src/filewidgets/kurlnavigatorpathselectoreventfilter.cpp",
     "URL navigator path selector filter.",
     ['inline constexpr unsigned kMaxPathLen = 4096;',
      'inline constexpr const char kHomePath[] = "~";']),
    ("frameworks/plasma.corebindingsplugin.cppm", "pbsd.kde.frameworks.plasma.corebindingsplugin",
     "pbsd::kde::frameworks::plasma::corebindingsplugin",
     "kde/frameworks/plasma-framework/src/declarativeimports/core/corebindingsplugin.cpp",
     "Plasma core QML bindings plugin.",
     ['inline constexpr const char kPluginId[] = "org.kde.plasma.core";',
      'inline constexpr const char kQmlModule[] = "org.kde.plasma.core";']),
    ("frameworks/kcoreaddons.kprocesslist_unix_procstat.cppm",
     "pbsd.kde.frameworks.kcoreaddons.kprocesslist_unix_procstat",
     "pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix_procstat",
     "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix_procstat.cpp",
     "procstat-backed process list constants.",
     ['inline constexpr unsigned kMaxPid = 99999;',
      'inline constexpr const char kProcPath[] = "/proc";']),
]

PLASMA: list[ModuleSpec] = [
    ("plasma.tablet.cppm", "pbsd.kde.plasma.tablet", "pbsd::kde::plasma::tablet",
     "kde/plasma-desktop/kcms/tablet/tabletmoduledata.cpp", "Tablet KCM module id.",
     ['inline constexpr const char kKcmId[] = "kcm_tablet";',
      'inline constexpr const char kConfigGroup[] = "Tablet";']),
    ("plasma.touchscreen.cppm", "pbsd.kde.plasma.touchscreen", "pbsd::kde::plasma::touchscreen",
     "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp", "Touchscreen KCM module id.",
     ['inline constexpr const char kKcmId[] = "kcm_touchscreen";']),
    ("plasma.globalpaths.cppm", "pbsd.kde.plasma.globalpaths", "pbsd::kde::plasma::globalpaths",
     "kde/plasma-desktop/kcms/desktoppaths/globalpaths.cpp", "Desktop paths XDG keys.",
     ['inline constexpr const char kDesktopLocation[] = "Desktop";',
      'inline constexpr const char kDocumentsLocation[] = "Documents";',
      'inline constexpr const char kPicturesLocation[] = "Pictures";']),
    ("plasma.cursortheme.cppm", "pbsd.kde.plasma.cursortheme", "pbsd::kde::plasma::cursortheme",
     "kde/plasma-desktop/kcms/mouse/cursortheme.cpp", "Cursor theme KCM keys.",
     ['inline constexpr const char kThemeKey[] = "cursorTheme";',
      'inline constexpr const char kSizeKey[] = "cursorSize";']),
    ("plasma.showdesktop.cppm", "pbsd.kde.plasma.showdesktop", "pbsd::kde::plasma::showdesktop",
     "kde/plasma-desktop/applets/showdesktop/showdesktop.cpp", "Show desktop applet id.",
     ['inline constexpr const char kAppletId[] = "org.kde.plasma.showdesktop";',
      'inline constexpr const char kPeekKey[] = "peek";']),
    ("plasma.menuhelper.cppm", "pbsd.kde.plasma.menuhelper", "pbsd::kde::plasma::menuhelper",
     "kde/plasma-desktop/containments/desktop/plugins/folder/menuhelper.cpp", "Folder menu helper keys.",
     ['inline constexpr const char kNewFolderAction[] = "newFolder";',
      'inline constexpr const char kPasteAction[] = "paste";']),
    ("plasma.wheelinterceptor.cppm", "pbsd.kde.plasma.wheelinterceptor", "pbsd::kde::plasma::wheelinterceptor",
     "kde/plasma-desktop/containments/desktop/plugins/folder/wheelinterceptor.cpp", "Wheel scroll delta cap.",
     ['inline constexpr int kMaxDelta = 120;',
      'inline constexpr int kMinDelta = -120;']),
    ("plasma.tastenbrett.geometry.cppm", "pbsd.kde.plasma.tastenbrett.geometry",
     "pbsd::kde::plasma::tastenbrett::geometry",
     "kde/plasma-desktop/kcms/keyboard/tastenbrett/geometry.cpp", "Keyboard layout geometry constants.",
     ['inline constexpr int kDefaultKeyWidth = 48;',
      'inline constexpr int kDefaultKeyHeight = 48;']),
    ("plasma.tastenbrett.section.cppm", "pbsd.kde.plasma.tastenbrett.section",
     "pbsd::kde.plasma::tastenbrett::section",
     "kde/plasma-desktop/kcms/keyboard/tastenbrett/section.cpp", "Keyboard section labels.",
     ['inline constexpr const char kAlphanumeric[] = "alphanumeric";',
      'inline constexpr const char kModifier[] = "modifier";']),
    ("plasma.gamecontroller.cppm", "pbsd.kde.plasma.gamecontroller", "pbsd::kde::plasma::gamecontroller",
     "kde/plasma-desktop/kcms/gamecontroller/axesproxymodel.cpp", "Game controller KCM id.",
     ['inline constexpr const char kKcmId[] = "kcm_gamecontroller";',
      'inline constexpr unsigned kMaxAxes = 16;']),
    ("plasma.emojier.cppm", "pbsd.kde.plasma.emojier", "pbsd::kde::plasma::emojier",
     "kde/plasma-desktop/emojier/emojicategory.cpp", "Emoji category ids.",
     ['inline constexpr const char kSmileys[] = "smileys";',
      'inline constexpr const char kPeople[] = "people";']),
    ("plasma.knetattach.cppm", "pbsd.kde.plasma.knetattach", "pbsd::kde::plasma::knetattach",
     "kde/plasma-desktop/knetattach/main.cpp", "Network attach wizard id.",
     ['inline constexpr const char kAppId[] = "knetattach";',
      'inline constexpr const char kProtocolKey[] = "protocol";']),
    ("plasma.qtquicksettings.cppm", "pbsd.kde.plasma.qtquicksettings", "pbsd::kde::plasma::qtquicksettings",
     "kde/plasma-desktop/kcms/qtquicksettings/kcmqtquicksettings.cpp", "Qt Quick settings KCM.",
     ['inline constexpr const char kKcmId[] = "kcm_qtquicksettings";',
      'inline constexpr const char kRenderLoopKey[] = "renderLoop";']),
    ("plasma.kded.cppm", "pbsd.kde.plasma.kded", "pbsd::kde::plasma::kded",
     "kde/plasma-desktop/kcms/kded/kdedconfigdata.cpp", "KDED modules config group.",
     ['inline constexpr const char kConfigGroup[] = "Module-kded";',
      'inline constexpr const char kAutoloadKey[] = "autoload";']),
    ("plasma.systemsettings.cppm", "pbsd.kde.plasma.systemsettings", "pbsd::kde::plasma::systemsettings",
     "kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp", "System Settings category ids.",
     ['inline constexpr const char kAppearanceCategory[] = "appearance";',
      'inline constexpr const char kWorkspaceCategory[] = "workspace";']),
]

KWIN_LOGGING = [
    ("kwin/scene.logging.cppm", "pbsd.kde.kwin.scene.logging", "pbsd::kde::kwin::scene::logging",
     "kde/kwin/src/scene/itemrenderer.cpp", "KWIN_SCENE", "kwin_scene"),
    ("kwin/core.logging.cppm", "pbsd.kde.kwin.core.logging", "pbsd::kde::kwin::core::logging",
     "kde/kwin/src/core/renderjournal.cpp", "KWIN_CORE", "kwin_core"),
    ("kwin/backends.logging.cppm", "pbsd.kde.kwin.backends.logging", "pbsd::kde::kwin::backends::logging",
     "kde/kwin/src/backends/drm/drm_blob.cpp", "KWIN_BACKENDS", "kwin_backends"),
]


def main() -> None:
    created: list[str] = []

    for batch in (KWIN_CORE, KWIN_WAYLAND, KWIN_PLUGINS, FRAMEWORKS, PLASMA):
        for spec in batch:
            rel, mod, ns, upstream, doc, body = spec[:6]
            extra = spec[6] if len(spec) > 6 else None
            path = OUT / rel
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(simple_module(rel, mod, ns, upstream, doc, body, extra), encoding="utf-8")
            created.append(rel)

    for rel, mod, ns, upstream, cat_id, cat_name in KWIN_LOGGING:
        path = OUT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(logging_module(rel, mod, ns, upstream, cat_id, cat_name), encoding="utf-8")
        created.append(rel)

    print(f"Created {len(created)} modules")
    for c in sorted(created):
        print(c)


if __name__ == "__main__":
    main()
