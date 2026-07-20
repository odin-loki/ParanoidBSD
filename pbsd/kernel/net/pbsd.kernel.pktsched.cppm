module;
#include <cstdint>

export module pbsd.kernel.pktsched;

import pbsd.core;


/// Freestanding port of `net/pktsched/pktsched.c`.
export namespace pbsd::kernel::pktsched {

inline constexpr unsigned kDefaultQlen = 50;

struct QueueLimits {
    unsigned qlen{kDefaultQlen};
    unsigned drops{};
};

[[nodiscard]] inline Status validate_qlen(unsigned qlen) noexcept {
    return qlen > 0 && qlen <= 1024 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status record_drop(QueueLimits& q) noexcept {
    ++q.drops;
    return Status::Ok;
}

} // namespace pbsd::kernel::pktsched
