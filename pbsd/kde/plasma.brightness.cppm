export module pbsd.kde.plasma.brightness;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma brightness constants.
/// Upstream: kde/plasma-desktop/kcms/brightness/brightness.cpp
export namespace pbsd::kde::plasma::brightness {

inline constexpr const char kKcmId[] = "kcm_brightness";
inline constexpr int kMinPercent{1};
inline constexpr int kMaxPercent{100};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/brightness/brightness.cpp";
}

} // namespace pbsd::kde::plasma::brightness
