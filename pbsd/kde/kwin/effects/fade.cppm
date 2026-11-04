export module pbsd.kde.kwin.effects.fade;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 9 burst — KWin fade effect stub.
/// Upstream: kde/kwin/src/plugins/fade/main.cpp
export namespace pbsd::kde::kwin::effects::fade {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "kwin_effect_fade",
    "Fade",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/fade/main.cpp",
};

inline constexpr const char kEffectId[] = "fade";
inline constexpr const char kDisplayName[] = "Fade";
inline constexpr const char kPluginCategory[] = "Appearance";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/fade/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::fade
