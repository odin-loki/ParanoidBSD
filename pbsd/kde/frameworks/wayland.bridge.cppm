export module pbsd.kde.frameworks.wayland.bridge;

import pbsd.core;
import pbsd.kde.layershell.qwaylandlayershell;
import pbsd.kde.layershell.qwaylandlayersurface;

/// Burst 17 — layer-shell ↔ KWin/Plasma wiring via fully qualified wayland::layer:: names.
/// Upstream: kde/frameworks/layer-shell-qt/src/
export namespace pbsd::kde::frameworks::wayland::bridge {

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

struct LayerWire {
    const char* shell_iface{wayland::layer::kShellInterface};
    const char* surface_iface{wayland::layer::kSurfaceInterface};
    wayland::layer::ShellLayer default_layer{wayland::layer::ShellLayer::Top};
    int shell_version{::pbsd::kde::layershell::qwaylandlayershell::kVersion};
    int surface_version{::pbsd::kde::layershell::qwaylandlayersurface::kVersion};
};

[[nodiscard]] inline LayerWire default_wire() noexcept {
    return LayerWire{};
}

[[nodiscard]] inline Status validate_wire(const LayerWire& w) noexcept {
    if (w.shell_iface == nullptr || w.surface_iface == nullptr) {
        return Status::Invalid;
    }
    if (w.shell_version < 1 || w.surface_version < 1) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/";
}

} // namespace pbsd::kde::frameworks::wayland::bridge
