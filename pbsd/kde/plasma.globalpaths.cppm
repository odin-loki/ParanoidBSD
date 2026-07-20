export module pbsd.kde.plasma.globalpaths;

import pbsd.core;

/// Wave 3 pass 4 — Desktop paths XDG keys.
/// Upstream: kde/plasma-desktop/kcms/desktoppaths/globalpaths.cpp
export namespace pbsd::kde::plasma::globalpaths {

    inline constexpr const char kDesktopLocation[] = "Desktop";
    inline constexpr const char kDocumentsLocation[] = "Documents";
    inline constexpr const char kPicturesLocation[] = "Pictures";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/desktoppaths/globalpaths.cpp";
}

} // namespace pbsd::kde::plasma::globalpaths
