export module pbsd.kde.kwindowsystem.kwindowsystem;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kwindowsystem.cpp).
/// Upstream: kde/frameworks/kwindowsystem/src/platforms/xcb/kwindowsystem.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::kwindowsystem {

inline constexpr const char kNetWmName[] = "_NET_WM_NAME";
inline constexpr const char kNetWmIcon[] = "_NET_WM_ICON";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/platforms/xcb/kwindowsystem.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::kwindowsystem
