module;

export module pbsd.compositor.wayland.tearing_control_v1;

import pbsd.core;

/// Burst 15 — wp-tearing-control-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::tearing_control_v1 {

inline constexpr const char kInterface[] = "wp_tearing_control_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetTearingControl = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetTearingControl);
}

} // namespace pbsd::compositor::wayland::tearing_control_v1
