module;
#include <cstdint>

export module pbsd.geom.part;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/part/g_part.h — g_part probe priority / aliases.
export namespace pbsd::geom::part {

inline constexpr int kProbePriLow  = -10;
inline constexpr int kProbePriNorm = -5;
inline constexpr int kProbePriHigh = 0;

enum class Alias : unsigned int {
    AppleApfs     = 0,
    AppleBoot     = 1,
    AppleHfs      = 3,
    BiosBoot      = 10,
    Efi           = 11,
    Freebsd       = 12,
    FreebsdBoot   = 13,
    FreebsdSwap   = 14,
    FreebsdZfs    = 15,
    LinuxData     = 20,
    LinuxSwap     = 21,
    Mbr           = 30,
    Gpt           = 31,
};

[[nodiscard]] inline Status validate_probe_pri(int pri) noexcept {
    if (pri < kProbePriLow || pri > kProbePriHigh) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::part
