export module pbsd.kde.kwin.effects.showcompositing;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin effect factory constants.
/// Upstream: kde/kwin/src/plugins/showcompositing/main.cpp
export namespace pbsd::kde::kwin::effects::showcompositing {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_EFFECT_TOOLS",
    "kwin_effect_showcompositing",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/showcompositing/main.cpp",
};

inline constexpr const char kEffectId[] = "showcompositing";
inline constexpr const char kDisplayName[] = "Show Compositing";
inline constexpr const char kPluginCategory[] = "Tools";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/showcompositing/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::showcompositing
