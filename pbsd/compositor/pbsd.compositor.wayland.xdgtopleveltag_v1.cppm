module;

export module pbsd.compositor.wayland.xdgtopleveltag_v1;

import pbsd.core;

/// Burst 13 — xdg_toplevel_tag_v1 (native compositor).
export namespace pbsd::compositor::wayland::xdgtopleveltag_v1 {

inline constexpr const char kManagerInterface[] = "xdg_toplevel_tag_manager_v1";
inline constexpr const char kTagInterface[] = "xdg_toplevel_tag_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetToplevelTag = 1,
};

enum class TagRequest : unsigned char {
    Destroy = 0,
    SetDescription = 1,
    SetLabel = 2,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetToplevelTag);
}

[[nodiscard]] inline bool is_tag_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(TagRequest::SetLabel);
}

[[nodiscard]] inline Status validate_label_len(unsigned len) noexcept {
    return len > 0 && len <= 256 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::compositor::wayland::xdgtopleveltag_v1
