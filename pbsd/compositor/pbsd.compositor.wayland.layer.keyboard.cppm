module;

export module pbsd.compositor.wayland.layer.keyboard;

import pbsd.core;
import pbsd.compositor.wayland.layer;

/// Burst 16 wave 2 — layer-shell keyboard (native compositor).
/// Upstream: wlr-layer-shell-unstable-v1.xml
export namespace pbsd::compositor::wayland::layer::keyboard {

[[nodiscard]] inline Status validate_interactivity(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(::pbsd::compositor::wayland::layer::KeyboardInteractivity::OnDemand)
        ? Status::Ok : Status::Protocol;
}

} // namespace pbsd::compositor::wayland::layer::keyboard
