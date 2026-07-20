module;

export module pbsd.compositor.wayland.xdg_activation;

import pbsd.core;

/// Burst 14 — xdg-activation-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::xdg_activation {

inline constexpr const char kInterface[] = "xdg_activation_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetActivationToken = 1,
    Activate = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::Activate);
}

} // namespace pbsd::compositor::wayland::xdg_activation
