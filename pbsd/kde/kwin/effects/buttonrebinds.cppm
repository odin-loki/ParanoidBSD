export module pbsd.kde.kwin.effects.buttonrebinds;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/buttonrebinds/main.cpp
export namespace pbsd::kde::kwin::effects::buttonrebinds {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_MISCELLANEOUS",
    "kwin_effect_buttonrebinds",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/buttonrebinds/main.cpp",
};

inline constexpr const char kEffectId[] = "buttonrebinds";
inline constexpr const char kDisplayName[] = "buttonrebinds";
inline constexpr const char kPluginCategory[] = "Miscellaneous";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/buttonrebinds/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::buttonrebinds
