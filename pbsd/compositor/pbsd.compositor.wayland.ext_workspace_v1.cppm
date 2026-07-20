module;

export module pbsd.compositor.wayland.ext_workspace_v1;

import pbsd.core;

/// Burst 16 wave 2 — ext-workspace-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::ext_workspace_v1 {

inline constexpr const char kManagerInterface[] = "ext_workspace_manager_v1";
inline constexpr const char kHandleInterface[] = "ext_workspace_handle_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Commit = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Commit);
}

} // namespace pbsd::compositor::wayland::ext_workspace_v1
