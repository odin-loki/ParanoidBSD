export module pbsd.kde.kwin.desktopeffectsdata;

import pbsd.core;

/// Wave 3 pass 4 — Desktop effects KCM data group.
/// Upstream: kde/kwin/src/kcms/effects/desktopeffectsdata.cpp
export namespace pbsd::kde::kwin::desktopeffectsdata {

    inline constexpr const char kConfigGroup[] = "Plugins";
    inline constexpr const char kEnabledKey[] = "EnabledByDefault";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/kcms/effects/desktopeffectsdata.cpp";
}

} // namespace pbsd::kde::kwin::desktopeffectsdata
