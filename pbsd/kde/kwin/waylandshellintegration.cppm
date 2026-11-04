export module pbsd.kde.waylandshellintegration;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (waylandshellintegration.cpp).
/// Upstream: kde/kwin/src/waylandshellintegration.cpp
export namespace pbsd::kde::waylandshellintegration {

inline constexpr const char kXdgShell[] = "xdg_shell";
inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/waylandshellintegration.cpp";
}

} // namespace pbsd::kde::waylandshellintegration
