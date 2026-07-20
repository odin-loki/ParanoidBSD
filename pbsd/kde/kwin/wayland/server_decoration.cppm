export module pbsd.kde.server_decoration;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (server_decoration.cpp).
/// Upstream: kde/kwin/src/wayland/server_decoration.cpp
export namespace pbsd::kde::server_decoration {

inline constexpr const char kServerDecoration[] = "zxdg_decoration_manager_v1";
inline constexpr int kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/server_decoration.cpp";
}

} // namespace pbsd::kde::server_decoration
