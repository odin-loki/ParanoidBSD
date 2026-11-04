module;
#include <cstdint>

export module pbsd.compositor.wayland.seat;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_seat / wl_pointer / wl_keyboard — event/request opcodes.
export namespace pbsd::compositor::wayland::seat {

enum class SeatEvent : unsigned int {
    Capabilities = 0,
    Name         = 1,
};

enum class PointerRequest : unsigned int {
    SetCursor    = 0,
    Release      = 1,
};

enum class PointerEvent : unsigned int {
    Enter  = 0,
    Leave  = 1,
    Motion = 2,
    Button = 3,
    Axis   = 4,
    Frame  = 5,
    AxisSource = 6,
    AxisStop   = 7,
    AxisDiscrete = 8,
    AxisValue120 = 9,
};

enum class Capability : unsigned int {
    Pointer  = 1,
    Keyboard = 2,
    Touch    = 4,
};

[[nodiscard]] inline bool has_capability(unsigned caps, Capability c) noexcept {
    return (caps & static_cast<unsigned>(c)) != 0;
}

[[nodiscard]] inline Status validate_capabilities(unsigned caps) noexcept {
    if (caps == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::seat
