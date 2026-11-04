export module pbsd.kde.kwin.effects.magiclamp;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/magiclamp/main.cpp
export namespace pbsd::kde::kwin::effects::magiclamp {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_APPEARANCE",
    "kwin_effect_magiclamp",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/magiclamp/main.cpp",
};

inline constexpr const char kEffectId[] = "magiclamp";
inline constexpr const char kDisplayName[] = "Magic Lamp";
inline constexpr const char kPluginCategory[] = "Appearance";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/magiclamp/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::magiclamp
