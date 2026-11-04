export module pbsd.kde.kwin.effects.overview;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/overview/main.cpp
export namespace pbsd::kde::kwin::effects::overview {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_WINDOW_MANAGEMENT",
    "kwin_effect_overview",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/overview/main.cpp",
};

inline constexpr const char kEffectId[] = "overview";
inline constexpr const char kDisplayName[] = "Overview";
inline constexpr const char kPluginCategory[] = "Window Management";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/overview/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::overview
