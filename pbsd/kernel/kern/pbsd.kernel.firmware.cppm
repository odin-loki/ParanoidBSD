module;
#include <cstdint>

export module pbsd.kernel.firmware;

export import pbsd.core;

/// Freestanding port of `sys/firmware.h` / `kern/subr_firmware.c`.
export namespace pbsd::kernel::firmware {

inline constexpr unsigned kGetNoWarn = 0x0001;
inline constexpr unsigned kUnload = 0x0001;

struct Image {
    const char* name{nullptr};
    const void* data{nullptr};
    std::size_t size{};
    unsigned version{};
};

[[nodiscard]] inline Status validate_image(const Image& img) noexcept {
    if (img.name == nullptr || *img.name == '\0') {
        return Status::Invalid;
    }
    if (img.data == nullptr || img.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool no_warn(unsigned flags) noexcept {
    return (flags & kGetNoWarn) != 0;
}

} // namespace pbsd::kernel::firmware
