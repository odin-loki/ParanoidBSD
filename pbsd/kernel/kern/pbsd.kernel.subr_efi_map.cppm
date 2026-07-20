module;
#include <cstdint>

export module pbsd.kernel.subr_efi_map;

export import pbsd.core;

/// Freestanding port of `kern/subr_efi_map.c` — efi map helpers.
export namespace pbsd::kernel::subr_efi_map {

inline constexpr unsigned kEfiMapPages = 4096;

[[nodiscard]] inline Status validate_pages(unsigned pages) noexcept {
    return pages <= kEfiMapPages ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::subr_efi_map
