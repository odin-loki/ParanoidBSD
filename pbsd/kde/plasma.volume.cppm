export module pbsd.kde.plasma.volume;

import pbsd.core;

/// Wave 9 burst — Plasma volume applet constants.
/// Upstream: kde/plasma-workspace/applets/volume/volume.cpp
export namespace pbsd::kde::plasma::volume {

inline constexpr int kDefaultVolume = 50;
inline constexpr int kMaxVolume = 100;
inline constexpr int kSliderWidth = 120;

[[nodiscard]] inline Status validate_volume(int v) noexcept {
    if (v < 0 || v > kMaxVolume) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/applets/volume/volume.cpp";
}

} // namespace pbsd::kde::plasma::volume
