export module pbsd.kde.kwin.effects.showfps;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/showfps/main.cpp
export namespace pbsd::kde::kwin::effects::showfps {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_TOOLS",
    "kwin_effect_showfps",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/showfps/main.cpp",
};

inline constexpr const char kEffectId[] = "showfps";
inline constexpr const char kDisplayName[] = "Show FPS";
inline constexpr const char kPluginCategory[] = "Tools";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/showfps/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::showfps
