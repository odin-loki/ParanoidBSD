export module pbsd.kde.kwin.effects.zoom;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/zoom/main.cpp
export namespace pbsd::kde::kwin::effects::zoom {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_zoom",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/zoom/main.cpp",
};

inline constexpr const char kEffectId[] = "zoom";
inline constexpr const char kDisplayName[] = "Zoom";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/zoom/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::zoom
