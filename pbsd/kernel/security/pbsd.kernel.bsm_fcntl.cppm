module;
#include <cstdint>

export module pbsd.kernel.bsm_fcntl;

import pbsd.core;

/// Freestanding port of `security/audit/bsm_fcntl.c` — BSM fcntl command mappings.
export namespace pbsd::kernel::bsm_fcntl {

inline constexpr int kNoLocalMapping = -600;
inline constexpr int kFGetfl = 3;
inline constexpr int kFSetfl = 4;
inline constexpr int kFGetlk = 7;
inline constexpr int kFSetlk = 8;
inline constexpr unsigned kBsmFGetfl = 1;
inline constexpr unsigned kBsmFSetfl = 2;
inline constexpr unsigned kBsmFGetlk = 7;
inline constexpr unsigned kBsmFSetlk = 8;

[[nodiscard]] inline int lookup_local(unsigned bsm_cmd) noexcept {
    switch (bsm_cmd) {
    case kBsmFGetfl: return kFGetfl;
    case kBsmFSetfl: return kFSetfl;
    case kBsmFGetlk: return kFGetlk;
    case kBsmFSetlk: return kFSetlk;
    default: return kNoLocalMapping;
    }
}

[[nodiscard]] inline Status validate_cmd(unsigned bsm_cmd) noexcept {
    return lookup_local(bsm_cmd) != kNoLocalMapping ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::bsm_fcntl
