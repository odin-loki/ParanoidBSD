export module pbsd.kde.plasma.tastenbrett.row;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (row.cpp).
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/row.cpp
export namespace pbsd::kde::plasma::tastenbrett::row {

inline constexpr unsigned kMaxKeysPerRow = 20;
inline constexpr unsigned kKeySpacing = 4;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/row.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::row
