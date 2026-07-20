module;
#include <cstdint>

export module pbsd.kernel.sched_ule;

import pbsd.core;

/// Freestanding port of `kern/sched_ule.c` — ULE scheduler constants.
export namespace pbsd::kernel::sched_ule {

inline constexpr unsigned kRunqBuckets = 100;
inline constexpr unsigned kInteractiveSlice = 20;

[[nodiscard]] inline Status validate_priority(int pri) noexcept {
    if (pri < 0 || pri >= static_cast<int>(kRunqBuckets)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(int pri) noexcept {
    if (validate_priority(pri) != Status::Ok) {
        return 0;
    }
    return static_cast<unsigned>(pri);
}

} // namespace pbsd::kernel::sched_ule
