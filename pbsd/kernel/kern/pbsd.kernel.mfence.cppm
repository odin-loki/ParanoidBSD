module;
#include <cstdint>

export module pbsd.kernel.mfence;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_mfence.c — multi-CPU fence / barrier scaffold.
export namespace pbsd::kernel::mfence {

enum class Kind : unsigned char {
    Load = 0,
    Store = 1,
    Full = 2,
};

struct Epoch {
    std::uint64_t seq{};
    bool armed{false};
};

[[nodiscard]] inline Status arm(Epoch& e, Kind /*k*/) noexcept {
    ++e.seq;
    e.armed = true;
    return Status::Ok;
}

[[nodiscard]] inline Status sync(Epoch& e) noexcept {
    if (!e.armed) {
        return Status::Invalid;
    }
    e.armed = false;
    return Status::Ok;
}

} // namespace pbsd::kernel::mfence
