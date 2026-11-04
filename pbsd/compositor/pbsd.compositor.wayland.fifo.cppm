module;

export module pbsd.compositor.wayland.fifo;

import pbsd.core;

/// Burst 16 — fifo-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::fifo {

inline constexpr const char kInterface[] = "wp_fifo_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    SetBarrier = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetBarrier);
}

} // namespace pbsd::compositor::wayland::fifo
