module;
#include <cstdint>

export module pbsd.kernel.subr_rangeset;

import pbsd.core;

/// Freestanding port of `kern/subr_rangeset.c` — range set insert helpers.
export namespace pbsd::kernel::subr_rangeset {

[[nodiscard]] inline Status validate_insert(std::uint64_t start, std::uint64_t end) noexcept {
    if (end < start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool empty_range(std::uint64_t start, std::uint64_t end) noexcept {
    return start > end;
}

} // namespace pbsd::kernel::subr_rangeset
