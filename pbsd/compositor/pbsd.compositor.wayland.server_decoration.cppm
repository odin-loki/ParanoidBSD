module;

export module pbsd.compositor.wayland.server_decoration;

import pbsd.core;

/// Burst 13 — org_kde_kwin_server_decoration (native compositor).
export namespace pbsd::compositor::wayland::server_decoration {

inline constexpr const char kManagerInterface[] = "org_kde_kwin_server_decoration_manager";
inline constexpr const char kDecorationInterface[] = "org_kde_kwin_server_decoration";
inline constexpr int kVersion = 1;

enum class Mode : unsigned char {
    None = 0,
    Server = 1,
};

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Create = 1,
};

enum class DecorationRequest : unsigned char {
    Destroy = 0,
    RequestMode = 1,
    Release = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(Mode::Server) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Create);
}

[[nodiscard]] inline bool is_decoration_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DecorationRequest::Release);
}

} // namespace pbsd::compositor::wayland::server_decoration
