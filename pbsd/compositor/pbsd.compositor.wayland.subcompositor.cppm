module;

export module pbsd.compositor.wayland.subcompositor;

import pbsd.core;

/// Burst 13 — wl_subcompositor / wl_subsurface (native compositor).
export namespace pbsd::compositor::wayland::subcompositor {

inline constexpr const char kInterface[] = "wl_subcompositor";
inline constexpr int kVersion = 1;

enum class SubcompositorRequest : unsigned char {
    Destroy = 0,
    GetSubsurface = 1,
};

enum class SubsurfaceRequest : unsigned char {
    Destroy = 0,
    SetPosition = 1,
    PlaceAbove = 2,
    PlaceBelow = 3,
    SetSync = 4,
    SetDesync = 5,
};

[[nodiscard]] inline bool is_subcompositor_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SubcompositorRequest::GetSubsurface);
}

[[nodiscard]] inline bool is_subsurface_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SubsurfaceRequest::SetDesync);
}

[[nodiscard]] inline Status validate_position(int x, int y) noexcept {
    if (x < -32768 || x > 32767 || y < -32768 || y > 32767) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::subcompositor
