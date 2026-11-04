export module pbsd.kde.kwin.effects.kscreen;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/kscreen/main.cpp
export namespace pbsd::kde::kwin::effects::kscreen {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_APPEARANCE",
    "kwin_effect_kscreen",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/kscreen/main.cpp",
};

inline constexpr const char kEffectId[] = "kscreen";
inline constexpr const char kDisplayName[] = "Kscreen";
inline constexpr const char kPluginCategory[] = "Appearance";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kscreen/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::kscreen
