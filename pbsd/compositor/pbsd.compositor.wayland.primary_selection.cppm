module;

export module pbsd.compositor.wayland.primary_selection;

import pbsd.core;

/// Burst 14 — primary-selection-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::primary_selection {

inline constexpr const char kInterface[] = "zwp_primary_selection_device_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    SetSelection = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetSelection);
}

} // namespace pbsd::compositor::wayland::primary_selection
