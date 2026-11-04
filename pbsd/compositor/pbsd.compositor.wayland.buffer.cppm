module;
#include <cstdint>

export module pbsd.compositor.wayland.buffer;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_buffer / wl_shm — release and format opcodes.
export namespace pbsd::compositor::wayland::buffer {

enum class ShmFormat : unsigned int {
    CArgb8888 = 0,
    CXrgb8888 = 1,
    Crgb565   = 2,
};

enum class BufferRequest : unsigned int {
    Destroy  = 0,
    Release  = 0,
};

enum class ShmPoolRequest : unsigned int {
    CreateBuffer = 0,
    Destroy      = 1,
    Resize       = 2,
};

[[nodiscard]] inline Status validate_format(ShmFormat fmt) noexcept {
    if (static_cast<unsigned>(fmt) > static_cast<unsigned>(ShmFormat::Crgb565)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bytes_per_pixel(ShmFormat fmt) noexcept {
    switch (fmt) {
    case ShmFormat::CArgb8888:
    case ShmFormat::CXrgb8888:
        return 4;
    case ShmFormat::Crgb565:
        return 2;
    default:
        return 0;
    }
}

} // namespace pbsd::compositor::wayland::buffer
