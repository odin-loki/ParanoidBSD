export module pbsd.kde.kwin.effects.blur;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 5 — KWin blur compositing effect factory.
/// Upstream: kde/kwin/src/plugins/blur/main.cpp
export namespace pbsd::kde::kwin::effects::blur {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_BLUR",
    "kwin_blur",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/blur/main.cpp",
};

inline constexpr const char kEffectId[] = "blur";
inline constexpr const char kDisplayName[] = "Blur";
inline constexpr const char kPluginCategory[] = "Appearance";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/blur/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::blur
