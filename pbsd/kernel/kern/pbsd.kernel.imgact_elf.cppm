module;
#include <cstdint>

export module pbsd.kernel.imgact_elf;

import pbsd.core;

/// Freestanding port of `kern/imgact_elf.c` — ELF image activator constants.
export namespace pbsd::kernel::imgact_elf {

inline constexpr unsigned char kIdentMagic0 = 0x7F;
inline constexpr unsigned char kIdentMagic1 = 'E';
inline constexpr unsigned char kIdentMagic2 = 'L';
inline constexpr unsigned char kIdentMagic3 = 'F';
inline constexpr unsigned char kClass32 = 1;
inline constexpr unsigned char kClass64 = 2;

[[nodiscard]] inline Status validate_magic(unsigned char b0, unsigned char b1,
                                           unsigned char b2, unsigned char b3) noexcept {
    if (b0 == kIdentMagic0 && b1 == kIdentMagic1 && b2 == kIdentMagic2 && b3 == kIdentMagic3) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline Status validate_class(unsigned char cls) noexcept {
    switch (cls) {
    case kClass32:
    case kClass64:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::imgact_elf
