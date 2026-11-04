export module pbsd.kde.plasma.baloo;

import pbsd.core;

/// Wave 3 pass 3 — Baloo KCM data keys.
/// Upstream: kde/plasma-desktop/kcms/baloo/baloodata.cpp
export namespace pbsd::kde::plasma::baloo {

    inline constexpr const char kBalooGroup[] = "Baloo";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/baloo/baloodata.cpp";
}

} // namespace pbsd::kde::plasma::baloo
