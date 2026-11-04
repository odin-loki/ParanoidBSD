export module pbsd.kde.frameworks.kwindowsystem.waylandxdgactivationv1;

import pbsd.core;

/// Wave 3 pass 4 — xdg activation v1 protocol stub.
/// Upstream: kde/frameworks/kwindowsystem/src/platforms/wayland/waylandxdgactivationv1.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::waylandxdgactivationv1 {

    inline constexpr const char kProtocolName[] = "xdg_activation_v1";
    inline constexpr unsigned kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/platforms/wayland/waylandxdgactivationv1.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::waylandxdgactivationv1
