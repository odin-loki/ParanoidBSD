export module pbsd.kde.kwin.wayland.blur_contrast;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.blur;
import pbsd.kde.kwin.compositing;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 7 — KWin blur/contrast pipeline ↔ Aero glass tint glue.
/// Upstream: kde/kwin/src/plugins/blur/blur.cpp + contrast effect
export namespace pbsd::kde::kwin::wayland::blur_contrast {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_BLUR_CONTRAST",
    "kwin_blur_contrast",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/blur/blur.cpp",
};

struct PipelineParams {
    plasma::aero::BlurParams blur{};
    float contrast{1.05f};
    float saturation{1.15f};
    float noise{0.04f};
    bool enabled{true};
};

[[nodiscard]] inline PipelineParams panel_pipeline() noexcept {
    PipelineParams p{};
    p.blur = compositing::panel_blur();
    p.contrast = p.blur.contrast;
    p.saturation = p.blur.saturation;
    p.noise = p.blur.noise;
    return p;
}

[[nodiscard]] inline PipelineParams dialog_pipeline() noexcept {
    PipelineParams p{};
    p.blur = compositing::dialog_blur();
    p.contrast = p.blur.contrast;
    p.saturation = p.blur.saturation;
    p.noise = p.blur.noise;
    return p;
}

[[nodiscard]] inline PipelineParams from_builtin_blur() noexcept {
    PipelineParams p{};
    p.blur = blur::default_params();
    p.contrast = p.blur.contrast;
    p.saturation = p.blur.saturation;
    p.noise = p.blur.noise;
    return p;
}

[[nodiscard]] inline Status validate_contrast(float c) noexcept {
    if (c < 0.5f || c > 2.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return blur::upstream_path();
}

} // namespace pbsd::kde::kwin::wayland::blur_contrast
