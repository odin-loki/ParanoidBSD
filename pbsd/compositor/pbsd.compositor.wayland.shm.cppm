module;

export module pbsd.compositor.wayland.shm;

import pbsd.core;

/// Burst 15 — wayland.xml (native compositor).
export namespace pbsd::compositor::wayland::shm {

inline constexpr const char kInterface[] = "wl_shm";
inline constexpr int kVersion = 1;

enum class Format : unsigned char {
    Argb8888 = 0,
    Xrgb8888 = 1,
};

[[nodiscard]] inline bool is_format(unsigned fmt) noexcept {
    return fmt <= static_cast<unsigned>(Format::Xrgb8888);
}

} // namespace pbsd::compositor::wayland::shm
