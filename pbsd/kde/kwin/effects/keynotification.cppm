export module pbsd.kde.kwin.effects.keynotification;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/keynotification/main.cpp
export namespace pbsd::kde::kwin::effects::keynotification {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_MISCELLANEOUS",
    "kwin_effect_keynotification",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/keynotification/main.cpp",
};

inline constexpr const char kEffectId[] = "keynotification";
inline constexpr const char kDisplayName[] = "keynotification";
inline constexpr const char kPluginCategory[] = "Miscellaneous";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/keynotification/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::keynotification
