export module pbsd.kde.keyboard_shortcuts_inhibit;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (keyboard_shortcuts_inhibit.cpp).
/// Upstream: kde/kwin/src/wayland/keyboard_shortcuts_inhibit.cpp
export namespace pbsd::kde::keyboard_shortcuts_inhibit {

inline constexpr const char kShortcutsInhibit[] = "zwp_keyboard_shortcuts_inhibit_manager_v1";
inline constexpr int kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/keyboard_shortcuts_inhibit.cpp";
}

} // namespace pbsd::kde::keyboard_shortcuts_inhibit
