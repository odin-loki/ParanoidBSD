module;
#include <cstdint>

export module pbsd.kernel.imgact_elf64;

import pbsd.core;
import pbsd.kernel.imgact_elf;

/// Freestanding port of `kern/imgact_elf64.c` — 64-bit ELF activator helpers.
export namespace pbsd::kernel::imgact_elf64 {

inline constexpr unsigned kPhdrSize = 56;
inline constexpr unsigned kEhdrSize = 64;

[[nodiscard]] inline Status validate_ehdr_size(unsigned size) noexcept {
    return size >= kEhdrSize ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_class(unsigned char cls) noexcept {
    return cls == imgact_elf::kClass64 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::imgact_elf64
