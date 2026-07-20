module;
#include <cstdint>

export module pbsd.compositor.wayland.xdg;

import pbsd.core;

/// PROVENANCE: xdg-shell stable — toplevel/surface/positioner/decoration opcodes.
export namespace pbsd::compositor::wayland::xdg {

enum class ToplevelRequest : unsigned int {
    Destroy          = 0,
    SetParent        = 1,
    SetTitle         = 2,
    SetAppId         = 3,
    ShowWindowMenu   = 4,
    Move             = 5,
    Resize           = 6,
    SetMaxSize       = 7,
    SetMinSize       = 8,
    SetMaximized     = 9,
    UnsetMaximized   = 10,
    SetFullscreen    = 11,
    UnsetFullscreen  = 12,
    SetMinimized     = 13,
};

enum class PositionerRequest : unsigned int {
    Destroy       = 0,
    SetSize       = 1,
    SetAnchorRect = 2,
    SetAnchor     = 3,
    SetGravity    = 4,
    SetConstraintAdjustment = 5,
    SetOffset     = 6,
    SetReactive   = 7,
    SetParentSize = 8,
    SetParentConfigure = 9,
};

enum class DecorationRequest : unsigned int {
    Destroy  = 0,
    SetMode  = 1,
    UnsetMode = 2,
};

enum class WmCapability : unsigned int {
    WindowMenu     = 1,
    Maximize       = 2,
    Fullscreen     = 4,
    Minimize       = 8,
};

[[nodiscard]] inline bool is_toplevel_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ToplevelRequest::SetMinimized);
}

[[nodiscard]] inline bool is_positioner_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(PositionerRequest::SetParentConfigure);
}

[[nodiscard]] inline Status validate_wm_capabilities(unsigned caps) noexcept {
    if (caps > (static_cast<unsigned>(WmCapability::WindowMenu)
                | static_cast<unsigned>(WmCapability::Maximize)
                | static_cast<unsigned>(WmCapability::Fullscreen)
                | static_cast<unsigned>(WmCapability::Minimize))) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::xdg
