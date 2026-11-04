export module pbsd.kde.frameworks.plasma.bridge;

import pbsd.core;
import pbsd.kde.layershell.qwaylandlayershell;
import pbsd.kde.layershell.qwaylandlayersurface;

/// Burst 18 — Plasma ↔ layer-shell wiring via fully qualified wayland::layer:: names.
/// Upstream: kde/frameworks/plasma-framework/src/plasma/
export namespace pbsd::kde::frameworks::plasma::bridge {

namespace wayland::layer {

enum class ShellLayer : unsigned char {
    Background = 0,
    Bottom = 1,
    Top = 2,
    Overlay = 3,
};

inline constexpr const char kShellInterface[] = "zwlr_layer_shell_v1";
inline constexpr const char kSurfaceInterface[] = "zwlr_layer_surface_v1";

} // namespace wayland::layer

struct PlasmaLayerWire {
    const char* shell_iface{wayland::layer::kShellInterface};
    const char* surface_iface{wayland::layer::kSurfaceInterface};
    wayland::layer::ShellLayer panel_layer{wayland::layer::ShellLayer::Top};
    int shell_version{::pbsd::kde::layershell::qwaylandlayershell::kVersion};
    int surface_version{::pbsd::kde::layershell::qwaylandlayersurface::kVersion};
};

[[nodiscard]] inline PlasmaLayerWire default_wire() noexcept {
    return PlasmaLayerWire{};
}

[[nodiscard]] inline Status validate_wire(const PlasmaLayerWire& w) noexcept {
    if (w.shell_iface == nullptr || w.surface_iface == nullptr) {
        return Status::Invalid;
    }
    if (w.shell_version < 1 || w.surface_version < 1) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/";
}

} // namespace pbsd::kde::frameworks::plasma::bridge
