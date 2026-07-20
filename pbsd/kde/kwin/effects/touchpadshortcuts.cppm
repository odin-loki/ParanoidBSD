export module pbsd.kde.kwin.effects.touchpadshortcuts;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/touchpadshortcuts/main.cpp
export namespace pbsd::kde::kwin::effects::touchpadshortcuts {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_MISCELLANEOUS",
    "kwin_effect_touchpadshortcuts",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/touchpadshortcuts/main.cpp",
};

inline constexpr const char kEffectId[] = "touchpadshortcuts";
inline constexpr const char kDisplayName[] = "touchpadshortcuts";
inline constexpr const char kPluginCategory[] = "Miscellaneous";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/touchpadshortcuts/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::touchpadshortcuts
