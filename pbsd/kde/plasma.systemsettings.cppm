export module pbsd.kde.plasma.systemsettings;

import pbsd.core;

/// Wave 3 pass 4 — System Settings category ids.
/// Upstream: kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp
export namespace pbsd::kde::plasma::systemsettings {

    inline constexpr const char kAppearanceCategory[] = "appearance";
    inline constexpr const char kWorkspaceCategory[] = "workspace";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp";
}

} // namespace pbsd::kde::plasma::systemsettings
