module;
#include <cstdint>

export module pbsd.kernel.bsm_errno;

import pbsd.core;

/// Freestanding port of `security/audit/bsm_errno.c` — BSM errno mappings.
export namespace pbsd::kernel::bsm_errno {

inline constexpr int kNoLocalMapping = -600;

struct Mapping {
    int bsm_errno{};
    int local_errno{};
};

inline constexpr Mapping kTable[] = {
    {1, 1},
    {2, 2},
    {13, 13},
    {22, 22},
    {28, 28},
};

[[nodiscard]] inline unsigned table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTable) / sizeof(kTable[0]));
}

[[nodiscard]] inline int lookup_local(int bsm_errno) noexcept {
    for (unsigned i = 0; i < table_size(); ++i) {
        if (kTable[i].bsm_errno == bsm_errno) {
            return kTable[i].local_errno;
        }
    }
    return kNoLocalMapping;
}

[[nodiscard]] inline Status validate_bsm(int bsm_errno) noexcept {
    return lookup_local(bsm_errno) != kNoLocalMapping ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::bsm_errno
