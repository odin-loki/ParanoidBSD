module;
#include <cstdint>

export module pbsd.kernel.subr_compressor;

export import pbsd.core;

/// Freestanding port of `kern/subr_compressor.c` — compressor helpers.
export namespace pbsd::kernel::subr_compressor {

inline constexpr unsigned kCompEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kCompEnabled) != 0;
}

} // namespace pbsd::kernel::subr_compressor
