export module pbsd.kde.kwin.effects.pbsd_aero_blur;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;
import pbsd.kde.kwin.effects.registry;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 7 — pbsd_aero_blur KWin effect config from blur-effect.json.
/// Upstream: pbsd/theme/kwin-effects/pbsd_aero_blur/contents/code/pbsd_aero_blur.cpp
export namespace pbsd::kde::kwin::effects::pbsd_aero_blur {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_PBSD_AERO_BLUR",
    "kwin_pbsd_aero_blur",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "pbsd/theme/kwin-effects/pbsd_aero_blur/contents/code/pbsd_aero_blur.cpp",
};

inline constexpr const char kEffectId[] = "pbsd_aero_blur";
inline constexpr const char kMetadataPath[] = "kwin-effects/pbsd_aero_blur/metadata.json";
inline constexpr const char kConfigPath[] = "kwin/effects/pbsd_aero_blur/contents/config/pbsd_aero_blur.json";
inline constexpr const char kShaderDir[] = "kwin/effects/pbsd_aero_blur/contents/shaders";
inline constexpr bool kEnabledByDefault{true};

struct EffectConfig {
    plasma::aero::BlurParams panel{};
    plasma::aero::BlurParams dialog{};
    plasma::aero::BlurParams start_menu{};
    plasma::aero::BlurParams decoration{};
    plasma::aero::BlurParams glow{};
    plasma::aero::BlurParams system_link{};
    const char* decoration_plugin{plasma::aero::kDecorationId};
};

[[nodiscard]] inline EffectConfig from_aero_defaults() noexcept {
    EffectConfig cfg{};
    cfg.panel = compositing::panel_blur();
    cfg.dialog = compositing::dialog_blur();
    cfg.start_menu = compositing::start_menu_blur();
    cfg.decoration = compositing::decoration_blur();
    cfg.glow = compositing::glow_blur();
    cfg.system_link = compositing::system_link_blur();
    return cfg;
}

[[nodiscard]] inline bool is_registered_effect(const char* id) noexcept {
    if (id == nullptr) {
        return false;
    }
    return id[0] == registry::kPbsdAeroBlur[0]
        && id[4] == registry::kPbsdAeroBlur[4];
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/kwin-effects/pbsd_aero_blur/contents/code/pbsd_aero_blur.cpp";
}

} // namespace pbsd::kde::kwin::effects::pbsd_aero_blur
