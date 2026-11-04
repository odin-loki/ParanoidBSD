export module pbsd.kde.kwin.effects.hidecursor;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/hidecursor/main.cpp
export namespace pbsd::kde::kwin::effects::hidecursor {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_ACCESSIBILITY",
    "kwin_effect_hidecursor",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/hidecursor/main.cpp",
};

inline constexpr const char kEffectId[] = "hidecursor";
inline constexpr const char kDisplayName[] = "Hide Cursor";
inline constexpr const char kPluginCategory[] = "Accessibility";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/hidecursor/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::hidecursor
