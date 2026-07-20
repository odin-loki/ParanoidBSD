export module pbsd.kde.kwin.effects.magnifier;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/magnifier/main.cpp
export namespace pbsd::kde::kwin::effects::magnifier {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_magnifier",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/magnifier/main.cpp",
};

inline constexpr const char kEffectId[] = "magnifier";
inline constexpr const char kDisplayName[] = "Magnifier";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/magnifier/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::magnifier
