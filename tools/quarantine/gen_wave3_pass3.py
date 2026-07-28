#!/usr/bin/env python3
"""Generate Wave 3 pass-3 hand-port KDE C++23 modules."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
KDE = ROOT / "kde"
OUT = ROOT / "pbsd" / "kde"

EFFECT_PLUGINS = [
    "zoom", "hidecursor", "kscreen", "showpaint", "slideback", "mouseclick",
    "trackmouse", "diminactive", "touchpoints", "outputlocator", "screenedge",
    "thumbnailaside", "highlightwindow", "colorblindnesscorrection", "mousemark",
    "glide", "sheet", "slide", "invert", "blendchanges", "tileseditor",
    "fallapart", "magiclamp", "magnifier", "showfps", "systembell", "overview",
    "colorpicker", "windowview", "slidingpopups", "wobblywindows", "screentransform",
    "showcompositing", "startupfeedback", "shakecursor", "bouncekeys", "stickykeys",
    "slowkeys", "mousekeys", "keynotification", "buttonrebinds", "touchpadshortcuts",
    "nightlight", "screenshot", "gamecontroller", "krunner-integration", "eis",
]

FRAMEWORK_MODULES = [
    ("frameworks/kio.askuseractioninterface.cppm", "pbsd.kde.frameworks.kio.askuseractioninterface",
     "pbsd::kde::frameworks::kio::askuseractioninterface",
     "kde/frameworks/kio/src/core/askuseractioninterface.cpp",
     "KIO AskUserActionInterface IID stub.",
     ['inline constexpr const char kInterfaceId[] = "org.kde.kio.askuseractioninterface";']),
    ("frameworks/kio.koverlayiconplugin.cppm", "pbsd.kde.frameworks.kio.koverlayiconplugin",
     "pbsd::kde::frameworks::kio::koverlayiconplugin",
     "kde/frameworks/kio/src/core/koverlayiconplugin.cpp",
     "KOverlayIconPlugin base stub.",
     ['inline constexpr const char kPluginId[] = "org.kde.kio.overlayicon";']),
    ("frameworks/kio.dndpopupmenuplugin.cppm", "pbsd.kde.frameworks.kio.dndpopupmenuplugin",
     "pbsd::kde::frameworks::kio::dndpopupmenuplugin",
     "kde/frameworks/kio/src/gui/dndpopupmenuplugin.cpp",
     "DndPopupMenu QML plugin id.",
     ['inline constexpr const char kPluginId[] = "org.kde.kio.dndpopupmenu";']),
    ("frameworks/kio.jobtracker.cppm", "pbsd.kde.frameworks.kio.jobtracker",
     "pbsd::kde::frameworks::kio::jobtracker",
     "kde/frameworks/kio/src/core/jobtracker.cpp",
     "KIO job tracker interface.",
     ['inline constexpr const char kInterfaceId[] = "org.kde.kio.jobtracker";']),
    ("frameworks/kio.kabstractfileitemactionplugin.cppm",
     "pbsd.kde.frameworks.kio.kabstractfileitemactionplugin",
     "pbsd::kde::frameworks::kio::kabstractfileitemactionplugin",
     "kde/frameworks/kio/src/widgets/kabstractfileitemactionplugin.cpp",
     "Abstract file item action plugin.",
     ['inline constexpr const char kPluginId[] = "org.kde.kio.fileitemaction";']),
    ("frameworks/plasma.plasma.cppm", "pbsd.kde.frameworks.plasma.plasma",
     "pbsd::kde::frameworks::plasma::plasma",
     "kde/frameworks/plasma-framework/src/plasma/plasma.cpp",
     "Plasma Types registration stub.",
     ['inline constexpr const char kModuleName[] = "Plasma";']),
    ("frameworks/plasma.utils.cppm", "pbsd.kde.frameworks.plasma.utils",
     "pbsd::kde::frameworks::plasma::utils",
     "kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp",
     "PlasmaQuick utils constants.",
     ['inline constexpr const char kUtilsUri[] = "org.kde.plasma.utils";']),
    ("frameworks/plasma.containmentactions.cppm", "pbsd.kde.frameworks.plasma.containmentactions",
     "pbsd::kde::frameworks::plasma::containmentactions",
     "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp",
     "Containment actions package structure.",
     ['inline constexpr const char kPackageStructure[] = "Plasma/ContainmentActions";']),
    ("frameworks/kdecoration.bridge.cppm", "pbsd.kde.frameworks.kdecoration.bridge",
     "pbsd::kde::frameworks::kdecoration::bridge",
     "kde/frameworks/kdecoration/src/private/decorationbridge.cpp",
     "KDecoration private bridge stub.",
     ['inline constexpr const char kBridgeId[] = "org.kde.kdecoration.bridge";']),
    ("frameworks/kwindowsystem.plugininterface.cppm", "pbsd.kde.frameworks.kwindowsystem.plugininterface",
     "pbsd::kde::frameworks::kwindowsystem::plugininterface",
     "kde/frameworks/kwindowsystem/src/kwindowsystemplugininterface.cpp",
     "KWindowSystem plugin interface.",
     ['inline constexpr const char kInterfaceId[] = "org.kde.kwindowsystem.plugin";']),
    ("frameworks/layershell.shell.cppm", "pbsd.kde.frameworks.layershell.shell",
     "pbsd::kde::frameworks::layershell::shell",
     "kde/frameworks/layer-shell-qt/src/interfaces/shell.cpp",
     "Layer-shell Qt interface stub.",
     ['inline constexpr const char kInterfaceId[] = "org.kde.layershell";']),
]

KWIN_CORE = [
    ("kwin/plugin.cppm", "pbsd.kde.kwin.plugin", "pbsd::kde::kwin::plugin",
     "kde/kwin/src/plugin.cpp", "KWin PluginFactory stub.",
     ['inline constexpr const char kFactoryId[] = "org.kde.kwin.plugin";']),
    ("kwin/graphicsbufferallocator.cppm", "pbsd.kde.kwin.graphicsbufferallocator",
     "pbsd::kde::kwin::graphicsbufferallocator",
     "kde/kwin/src/core/graphicsbufferallocator.cpp", "Graphics buffer allocator constants.",
     ['inline constexpr unsigned kDefaultPoolSize{16};']),
    ("kwin/textinput.cppm", "pbsd.kde.kwin.textinput", "pbsd::kde::kwin::textinput",
     "kde/kwin/src/wayland/textinput.cpp", "Wayland text-input protocol stub.",
     ['inline constexpr const char kProtocolName[] = "zwp_text_input_v3";']),
    ("kwin/vsyncmonitor.cppm", "pbsd.kde.kwin.vsyncmonitor", "pbsd::kde::kwin::vsyncmonitor",
     "kde/kwin/src/utils/vsyncmonitor.cpp", "VSync monitor timing constants.",
     ['inline constexpr unsigned kDefaultRefreshHz{60};']),
    ("kwin/rootitem.cppm", "pbsd.kde.kwin.rootitem", "pbsd::kde::kwin::rootitem",
     "kde/kwin/src/scene/rootitem.cpp", "Scene root item layer id.",
     ['inline constexpr int kRootLayer{0};']),
]

PLASMA_DESKTOP = [
    ("plasma.libinputcommon.cppm", "pbsd.kde.plasma.libinputcommon",
     "pbsd::kde::plasma::libinputcommon",
     "kde/plasma-desktop/kcms/touchpad/backends/libinputcommon.cpp",
     "Touchpad libinput common constants.",
     ['inline constexpr const char kBackendName[] = "libinput";']),
    ("plasma.keyboard.cppm", "pbsd.kde.plasma.keyboard", "pbsd::kde::plasma::keyboard",
     "kde/plasma-desktop/kcms/keyboard/kcmmain.cpp", "Keyboard KCM module id.",
     ['inline constexpr const char kKcmId[] = "kcm_keyboard";']),
    ("plasma.baloo.cppm", "pbsd.kde.plasma.baloo", "pbsd::kde::plasma::baloo",
     "kde/plasma-desktop/kcms/baloo/baloodata.cpp", "Baloo KCM data keys.",
     ['inline constexpr const char kBalooGroup[] = "Baloo";']),
]


def sanitize_ns(name: str) -> str:
    return re.sub(r"[^a-zA-Z0-9_]", "_", name.replace("-", "_"))


def read_effect_meta(plugin: str) -> tuple[str, str]:
    meta_path = KDE / "kwin" / "src" / "plugins" / plugin / "metadata.json"
    if not meta_path.exists():
        return plugin, "Miscellaneous"
    data = json.loads(meta_path.read_text(encoding="utf-8"))
    kplugin = data.get("KPlugin", {})
    name = kplugin.get("Name", plugin)
    category = kplugin.get("Category", "Miscellaneous")
    return name, category


def effect_module(plugin: str) -> str:
    ns = sanitize_ns(plugin)
    mod = f"pbsd.kde.kwin.effects.{ns}"
    name, category = read_effect_meta(plugin)
    cat_upper = re.sub(r"[^A-Z0-9]", "_", category.upper())
    upstream = f"kde/kwin/src/plugins/{plugin}/main.cpp"
    return f"""export module {mod};

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: {upstream}
export namespace pbsd::kde::kwin::effects::{ns} {{

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{{
    "KWIN_EFFECT_{cat_upper}",
    "kwin_effect_{ns}",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "{upstream}",
}};

inline constexpr const char kEffectId[] = "{plugin}";
inline constexpr const char kDisplayName[] = "{name}";
inline constexpr const char kPluginCategory[] = "{category}";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace pbsd::kde::kwin::effects::{ns}
"""


def simple_module(rel: str, mod: str, ns: str, upstream: str, doc: str, body_lines: list[str],
                  extra_imports: list[str] | None = None) -> str:
    imports = ["import pbsd.core;"]
    if extra_imports:
        imports.extend(extra_imports)
    body = "\n".join(f"    {line}" for line in body_lines)
    return f"""export module {mod};

{chr(10).join(imports)}

/// Wave 3 pass 3 — {doc}
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


def main() -> None:
    created: list[str] = []

    effects_dir = OUT / "kwin" / "effects"
    effects_dir.mkdir(parents=True, exist_ok=True)
    for plugin in EFFECT_PLUGINS:
        rel = f"kwin/effects/{sanitize_ns(plugin)}.cppm"
        path = OUT / rel
        path.write_text(effect_module(plugin), encoding="utf-8")
        created.append(rel)

    for rel, mod, ns, upstream, doc, body in FRAMEWORK_MODULES:
        path = OUT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(simple_module(rel, mod, ns, upstream, doc, body), encoding="utf-8")
        created.append(rel)

    for rel, mod, ns, upstream, doc, body in KWIN_CORE:
        path = OUT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        extra = ["import pbsd.kde.kwin.logging.common;"] if "plugin" in rel else None
        path.write_text(simple_module(rel, mod, ns, upstream, doc, body, extra), encoding="utf-8")
        created.append(rel)

    for rel, mod, ns, upstream, doc, body in PLASMA_DESKTOP:
        path = OUT / rel
        path.write_text(simple_module(rel, mod, ns, upstream, doc, body), encoding="utf-8")
        created.append(rel)

    print(f"Created {len(created)} modules")
    for c in created:
        print(c)


if __name__ == "__main__":
    main()
