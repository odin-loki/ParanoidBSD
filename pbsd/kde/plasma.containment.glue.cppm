module;

export module pbsd.kde.plasma.containment.glue;

import pbsd.core;
import pbsd.kde.plasma.shell;
import pbsd.kde.plasma.panel;

/// Wave 3 pass 7 — Plasma containment type ↔ layer surface role glue.
/// Upstream: kde/plasma-framework/src/containment.cpp
export namespace pbsd::kde::plasma::containment::glue {

enum class ContainmentKind : unsigned char {
    Desktop,
    Panel,
    Custom,
};

enum class LayerRole : unsigned char {
    Desktop,
    Panel,
    Notification,
};

[[nodiscard]] inline ContainmentKind from_shell_name(const char* name) noexcept {
    if (name == nullptr) {
        return ContainmentKind::Custom;
    }
    if (name[0] == 'P') {
        return ContainmentKind::Panel;
    }
    if (name[0] == 'D') {
        return ContainmentKind::Desktop;
    }
    return ContainmentKind::Custom;
}

[[nodiscard]] inline LayerRole layer_role(ContainmentKind kind) noexcept {
    switch (kind) {
    case ContainmentKind::Panel:
        return LayerRole::Panel;
    case ContainmentKind::Desktop:
        return LayerRole::Desktop;
    default:
        return LayerRole::Notification;
    }
}

[[nodiscard]] inline int exclusive_zone(ContainmentKind kind) noexcept {
    return kind == ContainmentKind::Panel ? panel::kDefaultHeight : 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-framework/src/containment.cpp";
}

} // namespace pbsd::kde::plasma::containment::glue
