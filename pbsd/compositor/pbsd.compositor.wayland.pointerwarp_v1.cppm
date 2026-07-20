module;

export module pbsd.compositor.wayland.pointerwarp_v1;

import pbsd.core;

/// Burst 13 — wp_pointer_warp_v1 (native compositor).
export namespace pbsd::compositor::wayland::pointerwarp_v1 {

inline constexpr const char kInterface[] = "wp_pointer_warp_v1";
inline constexpr int kVersion = 1;

enum class WarpRequest : unsigned char {
    Destroy = 0,
    WarpPointer = 1,
};

[[nodiscard]] inline bool is_warp_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(WarpRequest::WarpPointer);
}

[[nodiscard]] inline Status validate_coords(int x, int y) noexcept {
    if (x < -32768 || x > 32767 || y < -32768 || y > 32767) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::pointerwarp_v1
