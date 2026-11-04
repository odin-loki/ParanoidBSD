export module pbsd.kde.kwin.effects.screenshot;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/screenshot/main.cpp
export namespace pbsd::kde::kwin::effects::screenshot {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_MISCELLANEOUS",
    "kwin_effect_screenshot",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/screenshot/main.cpp",
};

inline constexpr const char kEffectId[] = "screenshot";
inline constexpr const char kDisplayName[] = "screenshot";
inline constexpr const char kPluginCategory[] = "Miscellaneous";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/screenshot/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::screenshot
