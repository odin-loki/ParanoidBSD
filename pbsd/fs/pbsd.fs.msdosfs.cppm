module;
#include <cstdint>

export module pbsd.fs.msdosfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/msdosfs/fat.h — FAT cluster constants.
export namespace pbsd::fs::msdosfs {

inline constexpr unsigned kRoot = 0;
inline constexpr unsigned kFirst = 2;
inline constexpr unsigned kReserved = 0xfffffff6;
inline constexpr unsigned kBad = 0xfffffff7;
inline constexpr unsigned kEofs = 0xfffffff8;
inline constexpr unsigned kEofe = 0xffffffff;

inline constexpr unsigned kFat12Mask = 0x00000fff;
inline constexpr unsigned kFat16Mask = 0x0000ffff;
inline constexpr unsigned kFat32Mask = 0x0fffffff;

[[nodiscard]] inline Status validate_cluster(unsigned cl) noexcept {
    if (cl < kFirst && cl != kRoot) {
        return Status::Invalid;
    }
    if (cl >= kReserved && cl <= kBad) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::msdosfs
