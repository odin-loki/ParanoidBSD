module;

export module pbsd.compositor.wayland.security_context;

import pbsd.core;

/// Burst 16 — security-context-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::security_context {

inline constexpr const char kInterface[] = "wp_security_context_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateSecurityContext = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateSecurityContext);
}

} // namespace pbsd::compositor::wayland::security_context
