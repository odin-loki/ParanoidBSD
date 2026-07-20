export module pbsd.kde.frameworks.layershell.qwaylandxdgactivationv1;

import pbsd.core;

/// Wave 3 pass 4 — Layer-shell xdg activation.
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandxdgactivationv1.cpp
export namespace pbsd::kde::frameworks::layershell::qwaylandxdgactivationv1 {

    inline constexpr const char kInterfaceName[] = "xdg_activation_v1";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandxdgactivationv1.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::qwaylandxdgactivationv1
