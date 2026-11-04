export module pbsd.kde.kwin.effects.slide;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/slide/main.cpp
export namespace pbsd::kde::kwin::effects::slide {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_VIRTUAL_DESKTOP_SWITCHING_ANIMATION",
    "kwin_effect_slide",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/slide/main.cpp",
};

inline constexpr const char kEffectId[] = "slide";
inline constexpr const char kDisplayName[] = "Slide";
inline constexpr const char kPluginCategory[] = "Virtual Desktop Switching Animation";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/slide/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::slide
