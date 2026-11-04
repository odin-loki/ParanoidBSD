export module pbsd.kde.kwin.effects.dialogparent;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 9 burst — KWin dialog parent effect stub.
/// Upstream: kde/kwin/src/plugins/dialogparent/main.cpp
export namespace pbsd::kde::kwin::effects::dialogparent {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "kwin_effect_dialogparent",
    "Dialog Parent",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/dialogparent/main.cpp",
};

inline constexpr const char kEffectId[] = "dialogparent";
inline constexpr const char kDisplayName[] = "Dialog Parent";
inline constexpr const char kPluginCategory[] = "Accessibility";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/dialogparent/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::dialogparent
