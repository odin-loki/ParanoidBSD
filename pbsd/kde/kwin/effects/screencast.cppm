export module pbsd.kde.kwin.effects.screencast;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 5 — KWin screencast effect factory.
/// Upstream: kde/kwin/src/plugins/screencast/main.cpp
export namespace pbsd::kde::kwin::effects::screencast {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SCREENCAST",
    "kwin_screencast",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/screencast/main.cpp",
};

inline constexpr const char kEffectId[] = "screencast";
inline constexpr const char kDisplayName[] = "Screencast";
inline constexpr const char kPluginCategory[] = "Tools";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/screencast/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::screencast
