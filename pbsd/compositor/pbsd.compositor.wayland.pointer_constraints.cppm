module;

export module pbsd.compositor.wayland.pointer_constraints;

import pbsd.core;

/// Burst 16 — pointer-constraints-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::pointer_constraints {

inline constexpr const char kInterface[] = "zwp_pointer_constraints_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    LockPointer = 1,
    ConfinePointer = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::ConfinePointer);
}

} // namespace pbsd::compositor::wayland::pointer_constraints
