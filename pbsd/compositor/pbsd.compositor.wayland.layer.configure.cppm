module;

export module pbsd.compositor.wayland.layer.configure;

import pbsd.core;
import pbsd.compositor.wayland.layer;

/// Burst 16 wave 2 — layer-shell configure (native compositor).
/// Upstream: wlr-layer-shell-unstable-v1.xml
export namespace pbsd::compositor::wayland::layer::configure {

enum class ConfigureField : unsigned char {
    Width = 0,
    Height = 1,
    Serial = 2,
};

[[nodiscard]] inline Status validate_size(unsigned w, unsigned h) noexcept {
    if (w == 0 || h == 0 || w > 8192 || h > 8192) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::layer::configure
