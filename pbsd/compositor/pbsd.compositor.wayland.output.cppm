module;
#include <cstdint>

export module pbsd.compositor.wayland.output;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_output — mode/transform/subpixel enums.
export namespace pbsd::compositor::wayland::output {

enum class Subpixel : unsigned int {
    Unknown       = 0,
    None          = 1,
    HorizontalRgb = 2,
    HorizontalBgr = 3,
    VerticalRgb   = 4,
    VerticalBgr   = 5,
};

enum class Transform : unsigned int {
    Normal     = 0,
    Rotate90   = 1,
    Rotate180  = 2,
    Rotate270  = 3,
    Flipped    = 4,
    Flipped90  = 5,
    Flipped180 = 6,
    Flipped270 = 7,
};

enum class ModeFlag : unsigned int {
    Current = 0x1,
    Preferred = 0x2,
};

struct Mode {
    unsigned width{};
    unsigned height{};
    unsigned refresh_mhz{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_mode(Mode const& m) noexcept {
    if (m.width == 0 || m.height == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::output
