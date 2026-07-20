export module pbsd.kde.kwin.wayland.fixes;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (fixes.cpp).
/// Upstream: kde/kwin/src/wayland/fixes.cpp
export namespace pbsd::kde::kwin::wayland::fixes {

inline constexpr int kInterfaceVersion = 1;
inline constexpr const char kInterfaceName[] = "wl_fixes";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/fixes.cpp";
}

} // namespace pbsd::kde::kwin::wayland::fixes
