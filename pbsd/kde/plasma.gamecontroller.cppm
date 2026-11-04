export module pbsd.kde.plasma.gamecontroller;

import pbsd.core;

/// Wave 3 pass 4 — Game controller KCM id.
/// Upstream: kde/plasma-desktop/kcms/gamecontroller/axesproxymodel.cpp
export namespace pbsd::kde::plasma::gamecontroller {

    inline constexpr const char kKcmId[] = "kcm_gamecontroller";
    inline constexpr unsigned kMaxAxes = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/gamecontroller/axesproxymodel.cpp";
}

} // namespace pbsd::kde::plasma::gamecontroller
