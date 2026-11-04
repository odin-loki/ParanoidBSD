module;

export module pbsd.compositor.wayland.color_management;

import pbsd.core;

/// Burst 16 — color-management-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::color_management {

inline constexpr const char kInterface[] = "wp_color_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetOutput = 1,
    GetSurface = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurface);
}

} // namespace pbsd::compositor::wayland::color_management
