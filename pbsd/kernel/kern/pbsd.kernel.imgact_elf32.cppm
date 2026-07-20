module;
#include <cstdint>

export module pbsd.kernel.imgact_elf32;

import pbsd.core;
import pbsd.kernel.imgact_elf;

/// Freestanding port of `kern/imgact_elf32.c` — 32-bit ELF activator helpers.
export namespace pbsd::kernel::imgact_elf32 {

inline constexpr unsigned kPhdrSize = 32;
inline constexpr unsigned kEhdrSize = 52;

[[nodiscard]] inline Status validate_ehdr_size(unsigned size) noexcept {
    return size >= kEhdrSize ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_class(unsigned char cls) noexcept {
    return cls == imgact_elf::kClass32 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::imgact_elf32
