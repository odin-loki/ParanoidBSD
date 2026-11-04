module;

export module pbsd.compositor.wayland.idle_inhibit;

import pbsd.core;

/// Burst 14 — idle-inhibit-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::idle_inhibit {

inline constexpr const char kInterface[] = "zwp_idle_inhibit_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateInactive = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateInactive);
}

} // namespace pbsd::compositor::wayland::idle_inhibit
