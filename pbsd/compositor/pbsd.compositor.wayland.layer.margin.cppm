module;

export module pbsd.compositor.wayland.layer.margin;

import pbsd.core;
import pbsd.compositor.wayland.layer;

/// Burst 16 wave 2 — layer-shell margin (native compositor).
/// Upstream: wlr-layer-shell-unstable-v1.xml
export namespace pbsd::compositor::wayland::layer::margin {

struct Margin {
    int top{};
    int right{};
    int bottom{};
    int left{};
};

[[nodiscard]] inline Status validate_margin(const Margin& m) noexcept {
    if (m.top < -512 || m.bottom < -512 || m.left < -512 || m.right < -512) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::layer::margin
