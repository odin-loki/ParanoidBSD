export module pbsd.kde.frameworks.plasma.layer_surface;

import pbsd.core;
import pbsd.kde.plasma.panel;
import pbsd.kde.layershell.layershellintegration;

/// Wave 3 pass 7 — Plasma containment ↔ layer surface role glue.
/// Upstream: kde/frameworks/plasma/src/containment.cpp + layer-shell-qt
export namespace pbsd::kde::frameworks::plasma::layer_surface {

enum class Role : unsigned char {
    Panel,
    Desktop,
    FullscreenShell,
    Notification,
};

struct SurfaceBinding {
    Role role{Role::Panel};
    const char* integration_plugin{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin};
    const char* shell_interface{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface};
    int height{pbsd::kde::plasma::panel::kDefaultHeight};
    float opacity{pbsd::kde::plasma::panel::kDefaultOpacity};
};

[[nodiscard]] inline SurfaceBinding panel_binding() noexcept {
    SurfaceBinding b{};
    b.role = Role::Panel;
    b.height = pbsd::kde::plasma::panel::kDefaultHeight;
    b.opacity = pbsd::kde::plasma::panel::kDefaultOpacity;
    return b;
}

[[nodiscard]] inline bool is_panel_role(Role role) noexcept {
    return role == Role::Panel;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma/src/containment.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::layer_surface
