module;

export module pbsd.compositor.wayland.region;

import pbsd.core;

/// Burst 13 — wl_region rectangle limits (native compositor).
export namespace pbsd::compositor::wayland::region {

inline constexpr int kMaxRects = 64;
inline constexpr int kInvalidCoord = -1;

[[nodiscard]] inline Status validate_rect(int x, int y, int w, int h) noexcept {
    if (w < 0 || h < 0) {
        return Status::Invalid;
    }
    if (x == kInvalidCoord && y == kInvalidCoord) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rect_count(unsigned count) noexcept {
    return count <= static_cast<unsigned>(kMaxRects) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::compositor::wayland::region
