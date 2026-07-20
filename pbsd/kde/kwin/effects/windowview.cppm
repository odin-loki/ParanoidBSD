export module pbsd.kde.kwin.effects.windowview;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/windowview/main.cpp
export namespace pbsd::kde::kwin::effects::windowview {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_WINDOW_MANAGEMENT",
    "kwin_effect_windowview",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/windowview/main.cpp",
};

inline constexpr const char kEffectId[] = "windowview";
inline constexpr const char kDisplayName[] = "Present Windows";
inline constexpr const char kPluginCategory[] = "Window Management";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/windowview/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::windowview
