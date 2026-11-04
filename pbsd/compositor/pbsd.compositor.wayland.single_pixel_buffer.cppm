module;

export module pbsd.compositor.wayland.single_pixel_buffer;

import pbsd.core;

/// Burst 14 — single-pixel-buffer-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::single_pixel_buffer {

inline constexpr const char kInterface[] = "wp_single_pixel_buffer_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateU32RgbaBuffer = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateU32RgbaBuffer);
}

} // namespace pbsd::compositor::wayland::single_pixel_buffer
