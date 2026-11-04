module;
#include <cstdint>

export module pbsd.fs.vfs_mount;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_mount.c — mount reference counting.
export namespace pbsd::fs::vfs_mount {

enum class Flag : unsigned {
    ReadOnly = 0x0001,
    Sync     = 0x0002,
    NoExec   = 0x0004,
};

struct Mount {
    unsigned refs{1};
    unsigned flags{};
    bool mounted{false};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & ~(static_cast<unsigned>(Flag::ReadOnly) | static_cast<unsigned>(Flag::Sync)
                   | static_cast<unsigned>(Flag::NoExec))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status acquire(Mount& m) noexcept {
    if (!m.mounted) {
        return Status::Invalid;
    }
    ++m.refs;
    return Status::Ok;
}

[[nodiscard]] inline Status release(Mount& m) noexcept {
    if (m.refs == 0) {
        return Status::Invalid;
    }
    --m.refs;
    return Status::Ok;
}

} // namespace pbsd::fs::vfs_mount
