module;
#include <cstdint>

export module pbsd.zfs.dsl;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dsl_dataset.h — hold flags.
export namespace pbsd::zfs::dsl {

enum class HoldFlag : unsigned int {
    None      = 0,
    Recursive = 0x01,
    Clone     = 0x02,
    Temp      = 0x04,
};

enum class PropSource : unsigned char {
    None     = 0,
    Default  = 1,
    Local    = 2,
    Inherited = 3,
    Received = 4,
};

[[nodiscard]] inline Status validate_hold(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(HoldFlag::Clone))
        && (flags & static_cast<unsigned>(HoldFlag::Temp))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl
