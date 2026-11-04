export module pbsd.kde.plasma.baloo.baloodata;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (baloodata.cpp).
/// Upstream: kde/plasma-desktop/kcms/baloo/baloodata.cpp
export namespace pbsd::kde::plasma::baloo::baloodata {

inline constexpr const char kBalooGroup[] = "Basic Settings";
inline constexpr const char kEnabledKey[] = "enabled";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/baloo/baloodata.cpp";
}

} // namespace pbsd::kde::plasma::baloo::baloodata
