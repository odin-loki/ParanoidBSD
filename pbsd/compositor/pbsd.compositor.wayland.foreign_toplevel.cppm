module;

export module pbsd.compositor.wayland.foreign_toplevel;

import pbsd.core;

/// Burst 15 — ext-foreign-toplevel-list-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::foreign_toplevel {

inline constexpr const char kManagerInterface[] = "ext_foreign_toplevel_list_v1";
inline constexpr const char kHandleInterface[] = "ext_foreign_toplevel_handle_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Destroy);
}

} // namespace pbsd::compositor::wayland::foreign_toplevel
