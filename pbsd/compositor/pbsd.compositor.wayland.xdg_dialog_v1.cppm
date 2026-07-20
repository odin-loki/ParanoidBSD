module;

export module pbsd.compositor.wayland.xdg_dialog_v1;

import pbsd.core;

/// Burst 16 wave 2 — xdg-dialog-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::xdg_dialog_v1 {

inline constexpr const char kManagerInterface[] = "xdg_dialog_manager_v1";
inline constexpr const char kDialogInterface[] = "xdg_dialog_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetDialog = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetDialog);
}

} // namespace pbsd::compositor::wayland::xdg_dialog_v1
