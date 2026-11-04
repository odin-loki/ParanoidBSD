export module pbsd.kde.plasma.runners.locations;

import pbsd.core;

/// Burst 18 — Plasma locations runner constants.
/// Upstream: kde/plasma-workspace/runners/locations/locations.cpp
export namespace pbsd::kde::plasma::runners::locations {

inline constexpr const char kRunnerId[] = "locations";
inline constexpr const char kHomePrefix[] = "~";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/runners/locations/locations.cpp";
}

} // namespace pbsd::kde::plasma::runners::locations
