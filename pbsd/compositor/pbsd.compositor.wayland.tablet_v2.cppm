module;

export module pbsd.compositor.wayland.tablet_v2;

import pbsd.core;

/// Burst 15 — tablet-v2.xml (native compositor).
export namespace pbsd::compositor::wayland::tablet_v2 {

inline constexpr const char kInterface[] = "wl_tablet";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetSurface = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurface);
}

} // namespace pbsd::compositor::wayland::tablet_v2
