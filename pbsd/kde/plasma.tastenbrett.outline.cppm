export module pbsd.kde.plasma.tastenbrett.outline;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (outline.cpp).
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/outline.cpp
export namespace pbsd::kde::plasma::tastenbrett::outline {

inline constexpr unsigned kOutlineWidth = 2;
inline constexpr unsigned kCornerRadius = 4;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/outline.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::outline
