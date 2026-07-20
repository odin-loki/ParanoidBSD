export module pbsd.kde.kwin.blur;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — upstream BlurEffect factory constants.
/// Upstream: kde/kwin/src/plugins/blur/main.cpp
export namespace pbsd::kde::kwin::blur {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_BLUR",
    "kwin_blur",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/blur/main.cpp",
};

inline constexpr const char kMetadataFile[] = "metadata.json.stripped";
inline constexpr const char kEffectId[] = "blur";

[[nodiscard]] inline plasma::aero::BlurParams default_params() noexcept {
    return plasma::aero::default_blur();
}

[[nodiscard]] inline plasma::aero::BlurParams start_menu_params() noexcept {
    auto p = default_params();
    p.dialog_opacity = 0.78f;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams decoration_params() noexcept {
    auto p = default_params();
    p.panel_opacity = 0.78f;
    return p;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/blur/main.cpp";
}

} // namespace pbsd::kde::kwin::blur
