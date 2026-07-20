export module pbsd.kde.plasma.tablet;

import pbsd.core;

/// Wave 3 pass 4 — Tablet KCM module id.
/// Upstream: kde/plasma-desktop/kcms/tablet/tabletmoduledata.cpp
export namespace pbsd::kde::plasma::tablet {

    inline constexpr const char kKcmId[] = "kcm_tablet";
    inline constexpr const char kConfigGroup[] = "Tablet";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/tablet/tabletmoduledata.cpp";
}

} // namespace pbsd::kde::plasma::tablet
