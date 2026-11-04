export module pbsd.kde.kwin.killwindow;

import pbsd.core;

/// Wave 3 pass 4 — Kill-window helper shortcut id.
/// Upstream: kde/kwin/src/killwindow.cpp
export namespace pbsd::kde::kwin::killwindow {

    inline constexpr const char kShortcutId[] = "Kill Window";
    inline constexpr unsigned kGraceMs = 500;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/killwindow.cpp";
}

} // namespace pbsd::kde::kwin::killwindow
