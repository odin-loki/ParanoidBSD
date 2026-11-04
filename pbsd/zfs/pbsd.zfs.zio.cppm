module;
#include <cstdint>

export module pbsd.zfs.zio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO priority/flags.
export namespace pbsd::zfs::zio {

enum class Priority : unsigned char {
    Now      = 0,
    Sync     = 1,
    Async    = 2,
    Scrub    = 3,
    Idle     = 4,
};

enum class Flag : unsigned int {
    Read        = 0x00000001,
    Write       = 0x00000002,
    Free        = 0x00000004,
    Allocate    = 0x00000008,
    Defer       = 0x00000010,
    Nodata      = 0x00000020,
    SetError    = 0x00000040,
    Config      = 0x00000080,
    Rewrite     = 0x00000100,
    Raw         = 0x00000200,
};

enum class Stage : unsigned char {
    Open     = 0,
    Read     = 1,
    Write    = 2,
    Free     = 3,
    Done     = 4,
    Error    = 5,
};

[[nodiscard]] inline constexpr bool is_read(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Read)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Read))
        && (flags & static_cast<unsigned>(Flag::Write))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::zio
