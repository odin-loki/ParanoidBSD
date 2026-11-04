export module pbsd.kde.plasma.touchscreen.moduledata;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (touchscreenmoduledata.cpp).
/// Upstream: kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp
export namespace pbsd::kde::plasma::touchscreen::moduledata {

inline constexpr const char kTouchscreenGroup[] = "Touchscreen";
inline constexpr unsigned kMaxDevices = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp";
}

} // namespace pbsd::kde::plasma::touchscreen::moduledata
