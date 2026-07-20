module;

export module pbsd.compositor.wayland.cursor_shape;

import pbsd.core;

/// Burst 12 — wp_cursor_shape_manager_v1 (native compositor).
export namespace pbsd::compositor::wayland::cursor_shape {

inline constexpr const char kManagerInterface[] = "wp_cursor_shape_manager_v1";
inline constexpr const char kDeviceInterface[] = "wp_cursor_shape_device_v1";
inline constexpr int kVersion = 1;

enum class Shape : unsigned char {
    Default    = 1,
    Pointer    = 2,
    Text       = 3,
    Grab       = 4,
    NotAllowed = 5,
    Resize     = 6,
    Crosshair  = 7,
    Progress   = 8,
};

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Get     = 1,
};

enum class DeviceRequest : unsigned char {
    Destroy  = 0,
    SetShape = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Get);
}

[[nodiscard]] inline bool is_device_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DeviceRequest::SetShape);
}

[[nodiscard]] inline Status validate_shape(unsigned shape) noexcept {
    if (shape == 0 || shape > static_cast<unsigned>(Shape::Progress)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::cursor_shape
