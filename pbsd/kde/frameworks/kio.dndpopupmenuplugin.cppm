export module pbsd.kde.frameworks.kio.dndpopupmenuplugin;

import pbsd.core;

/// Wave 3 pass 3 — DndPopupMenu QML plugin id.
/// Upstream: kde/frameworks/kio/src/gui/dndpopupmenuplugin.cpp
export namespace pbsd::kde::frameworks::kio::dndpopupmenuplugin {

    inline constexpr const char kPluginId[] = "org.kde.kio.dndpopupmenu";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/gui/dndpopupmenuplugin.cpp";
}

} // namespace pbsd::kde::frameworks::kio::dndpopupmenuplugin
