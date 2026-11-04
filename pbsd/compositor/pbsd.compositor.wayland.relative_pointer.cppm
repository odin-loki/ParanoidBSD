module;

export module pbsd.compositor.wayland.relative_pointer;

import pbsd.core;

/// Burst 16 — relative-pointer-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::relative_pointer {

inline constexpr const char kInterface[] = "zwp_relative_pointer_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::Destroy);
}

} // namespace pbsd::compositor::wayland::relative_pointer
