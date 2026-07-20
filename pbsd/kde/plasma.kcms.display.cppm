export module pbsd.kde.plasma.kcms.display;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 17 — Plasma display KCM constants.
/// Upstream: kde/plasma-desktop/kcms/display/display.cpp
export namespace pbsd::kde::plasma::kcms::display {

inline constexpr const char kKcmId[] = "kcm_display";
inline constexpr unsigned kMinRefreshHz{30};
inline constexpr unsigned kMaxRefreshHz{360};

[[nodiscard]] inline Status validate_refresh(unsigned hz) noexcept {
    if (hz < kMinRefreshHz || hz > kMaxRefreshHz) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/display/display.cpp";
}

} // namespace pbsd::kde::plasma::kcms::display
