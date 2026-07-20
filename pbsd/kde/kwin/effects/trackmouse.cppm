export module pbsd.kde.kwin.effects.trackmouse;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/trackmouse/main.cpp
export namespace pbsd::kde::kwin::effects::trackmouse {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_trackmouse",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/trackmouse/main.cpp",
};

inline constexpr const char kEffectId[] = "trackmouse";
inline constexpr const char kDisplayName[] = "Track Mouse";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/trackmouse/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::trackmouse
