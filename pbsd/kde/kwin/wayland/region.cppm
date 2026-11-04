export module pbsd.kde.kwin.wayland.region;

import pbsd.core;

/// Wave 3 pass 4 — Wayland region rectangle limits.
/// Upstream: kde/kwin/src/wayland/region.cpp
export namespace pbsd::kde::kwin::wayland::region {

    inline constexpr int kMaxRects = 64;
    inline constexpr int kInvalidCoord = -1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/region.cpp";
}

} // namespace pbsd::kde::kwin::wayland::region
