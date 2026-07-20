module;
#include <cstdint>

export module pbsd.kernel.madvise;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_madvise.c — madvise(2) kernel helpers.
export namespace pbsd::kernel::madvise {

enum class Advice : unsigned char {
    Normal = 0,
    Random = 1,
    Sequential = 2,
    WillNeed = 3,
    DontNeed = 4,
};

struct Range {
    std::uintptr_t addr{};
    std::size_t len{};
};

[[nodiscard]] inline Status validate_range(Range const& r) noexcept {
    if (r.len == 0 || r.addr == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status apply(Range const& r, Advice a) noexcept {
    if (validate_range(r) != Status::Ok) {
        return Status::Invalid;
    }
    (void)a;
    return Status::Ok;
}

} // namespace pbsd::kernel::madvise
