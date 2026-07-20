module;

export module pbsd.compositor.wayland.data_device;

import pbsd.core;

/// Burst 14 — wayland.xml (native compositor).
export namespace pbsd::compositor::wayland::data_device {

inline constexpr const char kInterface[] = "wl_data_device";
inline constexpr int kVersion = 3;

enum class Request : unsigned char {
    Destroy = 0,
    StartDrag = 1,
    SetSelection = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetSelection);
}

} // namespace pbsd::compositor::wayland::data_device
