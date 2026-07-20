export module pbsd.kde.kwin.effects.mousemark;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/mousemark/main.cpp
export namespace pbsd::kde::kwin::effects::mousemark {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_APPEARANCE",
    "kwin_effect_mousemark",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/mousemark/main.cpp",
};

inline constexpr const char kEffectId[] = "mousemark";
inline constexpr const char kDisplayName[] = "Mouse Mark";
inline constexpr const char kPluginCategory[] = "Appearance";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/mousemark/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::mousemark
