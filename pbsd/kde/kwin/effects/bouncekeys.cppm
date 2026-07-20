export module pbsd.kde.kwin.effects.bouncekeys;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/bouncekeys/main.cpp
export namespace pbsd::kde::kwin::effects::bouncekeys {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_MISCELLANEOUS",
    "kwin_effect_bouncekeys",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/bouncekeys/main.cpp",
};

inline constexpr const char kEffectId[] = "bouncekeys";
inline constexpr const char kDisplayName[] = "bouncekeys";
inline constexpr const char kPluginCategory[] = "Miscellaneous";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/bouncekeys/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::bouncekeys
