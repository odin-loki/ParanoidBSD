export module pbsd.kde.kwin.effects.showpaint;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/showpaint/main.cpp
export namespace pbsd::kde::kwin::effects::showpaint {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_TOOLS",
    "kwin_effect_showpaint",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/showpaint/main.cpp",
};

inline constexpr const char kEffectId[] = "showpaint";
inline constexpr const char kDisplayName[] = "Show Paint";
inline constexpr const char kPluginCategory[] = "Tools";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/showpaint/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::showpaint
