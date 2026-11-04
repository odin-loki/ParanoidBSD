export module pbsd.kde.plasma.mouse.kapplymousetheme;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kapplymousetheme.cpp).
/// Upstream: kde/plasma-desktop/kcms/mouse/kapplymousetheme.cpp
export namespace pbsd::kde::plasma::mouse::kapplymousetheme {

inline constexpr const char kCursorThemeGroup[] = "Mouse";
inline constexpr const char kThemeKey[] = "cursorTheme";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/mouse/kapplymousetheme.cpp";
}

} // namespace pbsd::kde::plasma::mouse::kapplymousetheme
