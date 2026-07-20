export module pbsd.kde.kwin.effects.glide;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/glide/main.cpp
export namespace pbsd::kde::kwin::effects::glide {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_WINDOW_OPEN_CLOSE_ANIMATION",
    "kwin_effect_glide",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/glide/main.cpp",
};

inline constexpr const char kEffectId[] = "glide";
inline constexpr const char kDisplayName[] = "Glide";
inline constexpr const char kPluginCategory[] = "Window Open/Close Animation";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/glide/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::glide
