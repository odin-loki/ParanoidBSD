export module pbsd.kde.plasma.mediacontroller;

import pbsd.core;

/// Wave 9 burst — Plasma media controller MPRIS roles.
/// Upstream: kde/plasma-workspace/applets/mediacontroller/mediacontroller.cpp
export namespace pbsd::kde::plasma::mediacontroller {

enum class PlaybackStatus : unsigned char {
    Playing,
    Paused,
    Stopped,
};

inline constexpr unsigned kDefaultPollMs = 1000;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/applets/mediacontroller/mediacontroller.cpp";
}

} // namespace pbsd::kde::plasma::mediacontroller
