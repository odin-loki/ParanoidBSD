export module pbsd.kde.plasma.tastenbrett.shape;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (shape.cpp).
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/shape.cpp
export namespace pbsd::kde::plasma::tastenbrett::shape {

inline constexpr unsigned kKeyWidth = 48;
inline constexpr unsigned kKeyHeight = 48;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/shape.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::shape
