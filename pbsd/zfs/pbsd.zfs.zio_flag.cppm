module;
#include <cstdint>

export module pbsd.zfs.zio_flag;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO_FLAG_* bits.
export namespace pbsd::zfs::zio_flag {

enum class Flag : unsigned long long {
    DontAggregate = 1ULL << 0,
    IoRepair      = 1ULL << 1,
    SelfHeal      = 1ULL << 2,
    Resilver      = 1ULL << 3,
    Scrub         = 1ULL << 4,
    ScanThread    = 1ULL << 5,
    Physical      = 1ULL << 6,
    Canfail       = 1ULL << 7,
    Speculative   = 1ULL << 8,
    ConfigWriter  = 1ULL << 9,
    DontRetry     = 1ULL << 10,
    Nodata        = 1ULL << 12,
    IoRetry       = 1ULL << 15,
    Probe         = 1ULL << 16,
};

enum class FailureMode : unsigned char {
    Wait = 0,
    Continue = 1,
    Panic = 2,
};

[[nodiscard]] inline constexpr bool flag_has(unsigned long long f, Flag bit) noexcept {
    return (f & static_cast<unsigned long long>(bit)) != 0;
}

[[nodiscard]] inline Status validate_failure_mode(FailureMode m) noexcept {
    switch (m) {
    case FailureMode::Wait:
    case FailureMode::Continue:
    case FailureMode::Panic:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::zfs::zio_flag
