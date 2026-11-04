module;

export module pbsd.compositor.wayland.xdg_decoration;

import pbsd.core;

/// Burst 13 — zxdg_decoration_manager_v1 (native compositor).
export namespace pbsd::compositor::wayland::xdg_decoration {

inline constexpr const char kManagerInterface[] = "zxdg_decoration_manager_v1";
inline constexpr const char kToplevelDecoration[] = "zxdg_toplevel_decoration_v1";
inline constexpr int kVersion = 1;

enum class Mode : unsigned char {
    ClientSide = 1,
    ServerSide = 2,
};

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetToplevelDecoration = 1,
};

enum class ToplevelRequest : unsigned char {
    Destroy = 0,
    SetMode = 1,
    UnsetMode = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    if (mode < static_cast<unsigned>(Mode::ClientSide) ||
        mode > static_cast<unsigned>(Mode::ServerSide)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetToplevelDecoration);
}

[[nodiscard]] inline bool is_toplevel_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ToplevelRequest::UnsetMode);
}

} // namespace pbsd::compositor::wayland::xdg_decoration
