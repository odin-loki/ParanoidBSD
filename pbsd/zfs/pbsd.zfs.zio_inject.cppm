module;
#include <cstdint>

export module pbsd.zfs.zio_inject;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/zio_inject.c — fault injection flags.
export namespace pbsd::zfs::zio_inject {

enum class Fault : unsigned char {
    None = 0,
    ReadErr = 1,
    WriteErr = 2,
    Checksum = 3,
    Delay = 4,
};

struct Injector {
    Fault fault{Fault::None};
    unsigned probability{}; // 0-1000 permille
    unsigned count{};
};

[[nodiscard]] inline Status configure(Injector& inj, Fault fault, unsigned permille) noexcept {
    if (permille > 1000) {
        return Status::Invalid;
    }
    inj.fault = fault;
    inj.probability = permille;
    inj.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline bool should_fault(const Injector& inj, unsigned roll) noexcept {
    if (inj.fault == Fault::None) {
        return false;
    }
    return roll < inj.probability;
}

} // namespace pbsd::zfs::zio_inject
