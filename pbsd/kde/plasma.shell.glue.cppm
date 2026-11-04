export module pbsd.kde.plasma.shell.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.containment.glue;
import pbsd.kde.plasma.shell;

/// Burst 12 — Plasma shell session ↔ containment/layer role glue.
/// Upstream: kde/plasma-workspace/shell/shell.cpp
export namespace pbsd::kde::plasma::shell::glue {

struct SessionBinding {
    const char* dbus_service{shell::kShellDBusService};
    const char* object_path{shell::kShellObjectPath};
    const char* theme_id{aero::kThemeId};
    containment::glue::ContainmentKind desktop_kind{containment::glue::ContainmentKind::Desktop};
    containment::glue::ContainmentKind panel_kind{containment::glue::ContainmentKind::Panel};
};

[[nodiscard]] inline SessionBinding default_binding() noexcept {
    return SessionBinding{};
}

[[nodiscard]] inline containment::glue::LayerRole layer_for_containment(
    const char* name) noexcept {
    return containment::glue::layer_role(containment::glue::from_shell_name(name));
}

[[nodiscard]] inline int exclusive_zone_for(const char* name) noexcept {
    return containment::glue::exclusive_zone(containment::glue::from_shell_name(name));
}

[[nodiscard]] inline bool is_panel_containment(const char* name) noexcept {
    return containment::glue::from_shell_name(name) == containment::glue::ContainmentKind::Panel;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return shell::upstream_path();
}

} // namespace pbsd::kde::plasma::shell::glue
