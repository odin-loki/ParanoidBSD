export module pbsd.kde.plasma.containment;

import pbsd.core;

/// Wave 3 — hand port constants (containment.cpp).
/// Upstream: kde/plasma-framework/src/plasma/containment.cpp
export namespace pbsd::kde::plasma::containment {

inline constexpr const char kContainmentTypeDesktop[] = "Desktop";
inline constexpr const char kContainmentTypePanel[] = "Panel";
inline constexpr unsigned kMaxApplets = 64;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-framework/src/plasma/containment.cpp";
}

} // namespace pbsd::kde::plasma::containment
