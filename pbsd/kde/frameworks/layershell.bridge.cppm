export module pbsd.kde.layershell.bridge;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;
import pbsd.kde.layershell.qwaylandlayershell;
import pbsd.kde.layershell.qwaylandlayersurface;

/// Burst 14 — layer-shell-qt sibling wiring via fully qualified names.
/// Upstream: kde/frameworks/layer-shell-qt/src/
export namespace pbsd::kde::frameworks::layershell::bridge {

struct IntegrationWire {
    const char* plugin{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin};
    const char* shell_surface{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface};
    const char* layer_shell_iface{
        ::pbsd::kde::layershell::qwaylandlayershell::kInterface};
    const char* layer_surface_iface{
        ::pbsd::kde::layershell::qwaylandlayersurface::kInterface};
    int shell_version{::pbsd::kde::layershell::qwaylandlayershell::kVersion};
    int surface_version{::pbsd::kde::layershell::qwaylandlayersurface::kVersion};
};

[[nodiscard]] inline IntegrationWire default_wire() noexcept {
    return IntegrationWire{};
}

[[nodiscard]] inline Status validate_wire(const IntegrationWire& w) noexcept {
    if (w.plugin == nullptr || w.shell_surface == nullptr) {
        return Status::Invalid;
    }
    if (w.shell_version < 1 || w.surface_version < 1) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::upstream_path();
}

} // namespace pbsd::kde::frameworks::layershell::bridge
