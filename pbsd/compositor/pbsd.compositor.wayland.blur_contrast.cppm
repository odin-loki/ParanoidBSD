module;

export module pbsd.compositor.wayland.blur_contrast;

import pbsd.core;
import pbsd.compositor.wayland;

/// Burst 13 — Aero blur/contrast pipeline (native compositor).
export namespace pbsd::compositor::wayland::blur_contrast {

struct PipelineParams {
    wayland::AeroDefaults wire{};
    float contrast{1.05f};
    float saturation{1.15f};
    bool enabled{true};
};

[[nodiscard]] inline PipelineParams panel_pipeline() noexcept {
    PipelineParams p{};
    p.wire = AeroDefaults{};
    p.contrast = 1.05f;
    p.saturation = p.wire.saturation_boost;
    return p;
}

[[nodiscard]] inline PipelineParams dialog_pipeline() noexcept {
    PipelineParams p{};
    p.wire = AeroDefaults{};
    p.wire.dialog_opacity = 0.78f;
    p.contrast = 1.08f;
    p.saturation = p.wire.saturation_boost;
    return p;
}

[[nodiscard]] inline Status validate_contrast(float c) noexcept {
    if (c < 0.5f || c > 2.0f) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::blur_contrast
