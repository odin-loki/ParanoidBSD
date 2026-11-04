module;
#include <cstdint>

export module pbsd.kernel.subr_coverage;

export import pbsd.core;

/// Freestanding port of `kern/subr_coverage.c` — coverage helpers.
export namespace pbsd::kernel::subr_coverage {

inline constexpr unsigned kCovMaxSites = 65536;

[[nodiscard]] inline Status validate_site(unsigned site) noexcept {
    return site < kCovMaxSites ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::subr_coverage
