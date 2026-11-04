export module pbsd.kde.kwin.effects.systembell;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/systembell/main.cpp
export namespace pbsd::kde::kwin::effects::systembell {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_systembell",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/systembell/main.cpp",
};

inline constexpr const char kEffectId[] = "systembell";
inline constexpr const char kDisplayName[] = "System Bell";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/systembell/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::systembell
