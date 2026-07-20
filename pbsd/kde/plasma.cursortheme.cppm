export module pbsd.kde.plasma.cursortheme;

import pbsd.core;

/// Wave 3 pass 4 — Cursor theme KCM keys.
/// Upstream: kde/plasma-desktop/kcms/mouse/cursortheme.cpp
export namespace pbsd::kde::plasma::cursortheme {

    inline constexpr const char kThemeKey[] = "cursorTheme";
    inline constexpr const char kSizeKey[] = "cursorSize";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/mouse/cursortheme.cpp";
}

} // namespace pbsd::kde::plasma::cursortheme
