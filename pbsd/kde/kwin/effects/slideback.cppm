export module pbsd.kde.kwin.effects.slideback;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/slideback/main.cpp
export namespace pbsd::kde::kwin::effects::slideback {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_FOCUS",
    "kwin_effect_slideback",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/slideback/main.cpp",
};

inline constexpr const char kEffectId[] = "slideback";
inline constexpr const char kDisplayName[] = "Slide Back";
inline constexpr const char kPluginCategory[] = "Focus";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/slideback/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::slideback
