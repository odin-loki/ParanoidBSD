export module pbsd.kde.plasma.showdesktop;

import pbsd.core;

/// Wave 3 pass 4 — Show desktop applet id.
/// Upstream: kde/plasma-desktop/applets/showdesktop/showdesktop.cpp
export namespace pbsd::kde::plasma::showdesktop {

    inline constexpr const char kAppletId[] = "org.kde.plasma.showdesktop";
    inline constexpr const char kPeekKey[] = "peek";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/showdesktop/showdesktop.cpp";
}

} // namespace pbsd::kde::plasma::showdesktop
