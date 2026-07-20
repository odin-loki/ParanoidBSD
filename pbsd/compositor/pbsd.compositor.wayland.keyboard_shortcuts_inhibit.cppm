module;

export module pbsd.compositor.wayland.keyboard_shortcuts_inhibit;

import pbsd.core;

/// Burst 13 — zwp_keyboard_shortcuts_inhibit_manager_v1 (native compositor).
export namespace pbsd::compositor::wayland::keyboard_shortcuts_inhibit {

inline constexpr const char kManagerInterface[] = "zwp_keyboard_shortcuts_inhibit_manager_v1";
inline constexpr const char kInhibitorInterface[] = "zwp_keyboard_shortcuts_inhibitor_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    InhibitShortcuts = 1,
};

enum class InhibitorRequest : unsigned char {
    Destroy = 0,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::InhibitShortcuts);
}

[[nodiscard]] inline bool is_inhibitor_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(InhibitorRequest::Destroy);
}

} // namespace pbsd::compositor::wayland::keyboard_shortcuts_inhibit
