export module pbsd.kde.frameworks.kirigami.units;

import pbsd.core;

/// Wave 9 burst — Kirigami grid unit constants.
/// Upstream: kde/frameworks/kirigami/src/units.cpp
export namespace pbsd::kde::frameworks::kirigami::units {

inline constexpr int kGridUnit = 8;
inline constexpr int kSmallSpacing = 4;
inline constexpr int kLargeSpacing = 16;
inline constexpr float kDefaultFontSize = 10.0f;

[[nodiscard]] inline int gridUnits(int n) noexcept {
    return n * kGridUnit;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kirigami/src/units.cpp";
}

} // namespace pbsd::kde::frameworks::kirigami::units
