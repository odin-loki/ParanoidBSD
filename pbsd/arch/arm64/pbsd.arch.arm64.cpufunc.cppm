module;
#include <cstdint>

export module pbsd.arch.arm64.cpufunc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/cpu.h — memory barrier kinds.
export namespace pbsd::arch::arm64::cpufunc {

enum class Barrier : unsigned char {
    Dmb = 0,
    Dsb = 1,
    Isb = 2,
};

enum class ShareDomain : unsigned char {
    NonShareable = 0,
    InnerShareable = 1,
    OuterShareable = 2,
    FullSystem = 3,
};

[[nodiscard]] inline Status validate_barrier(Barrier b) noexcept {
    switch (b) {
    case Barrier::Dmb:
    case Barrier::Dsb:
    case Barrier::Isb:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_domain(ShareDomain d) noexcept {
    switch (d) {
    case ShareDomain::NonShareable:
    case ShareDomain::InnerShareable:
    case ShareDomain::OuterShareable:
    case ShareDomain::FullSystem:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::arch::arm64::cpufunc
