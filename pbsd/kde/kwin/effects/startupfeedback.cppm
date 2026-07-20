export module pbsd.kde.kwin.effects.startupfeedback;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/startupfeedback/main.cpp
export namespace pbsd::kde::kwin::effects::startupfeedback {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_APPEARANCE",
    "kwin_effect_startupfeedback",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/startupfeedback/main.cpp",
};

inline constexpr const char kEffectId[] = "startupfeedback";
inline constexpr const char kDisplayName[] = "Startup Feedback";
inline constexpr const char kPluginCategory[] = "Appearance";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/startupfeedback/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::startupfeedback
