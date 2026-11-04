module;
#include <cstdint>

export module pbsd.kernel.subr_clockcalib;

export import pbsd.core;

/// Freestanding port of `kern/subr_clockcalib.c` — clockcalib helpers.
export namespace pbsd::kernel::subr_clockcalib {

inline constexpr unsigned kCalibIntervalMs = 1000;

[[nodiscard]] inline Status validate_interval(unsigned ms) noexcept {
    return ms > 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::subr_clockcalib
