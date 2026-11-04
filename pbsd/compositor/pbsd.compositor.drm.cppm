module;
#include <cstdint>

export module pbsd.compositor.drm;

import pbsd.core;

/// PROVENANCE: drm_fourcc.h — pixel format and modifier stubs for compositor.
export namespace pbsd::compositor::drm {

enum class Fourcc : unsigned int {
    Argb8888 = 0x34325241,
    Xrgb8888 = 0x34325258,
    Abgr8888 = 0x34324241,
    Rgb565 = 0x36314752,
    Nv12 = 0x3231564E,
};

enum class ModifierFlag : unsigned long long {
    Invalid = 0,
    Linear = 1ull << 0,
    Tiled = 1ull << 1,
    Scanout = 1ull << 2,
};

[[nodiscard]] inline Status validate_fourcc(Fourcc f) noexcept {
    switch (f) {
    case Fourcc::Argb8888:
    case Fourcc::Xrgb8888:
    case Fourcc::Abgr8888:
    case Fourcc::Rgb565:
    case Fourcc::Nv12:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool modifier_has(unsigned long long m, ModifierFlag bit) noexcept {
    return (m & static_cast<unsigned long long>(bit)) != 0;
}

} // namespace pbsd::compositor::drm
