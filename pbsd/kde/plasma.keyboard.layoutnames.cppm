export module pbsd.kde.plasma.keyboard.layoutnames;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (layoutnames.cpp).
/// Upstream: kde/plasma-desktop/kcms/keyboard/layoutnames.cpp
export namespace pbsd::kde::plasma::keyboard::layoutnames {

inline constexpr const char kLayoutGroup[] = "LayoutList";
inline constexpr unsigned kMaxLayoutNameLen = 64;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/layoutnames.cpp";
}

} // namespace pbsd::kde::plasma::keyboard::layoutnames
