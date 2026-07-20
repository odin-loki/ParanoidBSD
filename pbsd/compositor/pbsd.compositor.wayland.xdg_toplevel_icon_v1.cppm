module;

export module pbsd.compositor.wayland.xdg_toplevel_icon_v1;

import pbsd.core;

/// Burst 15 — xdg-toplevel-icon-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::xdg_toplevel_icon_v1 {

inline constexpr const char kManagerInterface[] = "xdg_toplevel_icon_manager_v1";
inline constexpr const char kIconInterface[] = "xdg_toplevel_icon_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetIcon = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetIcon);
}

} // namespace pbsd::compositor::wayland::xdg_toplevel_icon_v1
