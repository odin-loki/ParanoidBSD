export module pbsd.kde.plasma.touchscreen;

import pbsd.core;

/// Wave 3 pass 4 — Touchscreen KCM module id.
/// Upstream: kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp
export namespace pbsd::kde::plasma::touchscreen {

    inline constexpr const char kKcmId[] = "kcm_touchscreen";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp";
}

} // namespace pbsd::kde::plasma::touchscreen
