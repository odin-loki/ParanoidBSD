module;

export module pbsd.compositor.wayland.data_control;

import pbsd.core;

/// Burst 16 — wlr-data-control-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::data_control {

inline constexpr const char kManagerInterface[] = "zwlr_data_control_manager_v1";
inline constexpr int kVersion = 2;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetDataDevice = 1,
    GetPrimarySelectionDevice = 2,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetPrimarySelectionDevice);
}

} // namespace pbsd::compositor::wayland::data_control
