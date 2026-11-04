export module pbsd.kde.kwin.effects.shakecursor;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/shakecursor/main.cpp
export namespace pbsd::kde::kwin::effects::shakecursor {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_shakecursor",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/shakecursor/main.cpp",
};

inline constexpr const char kEffectId[] = "shakecursor";
inline constexpr const char kDisplayName[] = "Shake Cursor";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/shakecursor/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::shakecursor
