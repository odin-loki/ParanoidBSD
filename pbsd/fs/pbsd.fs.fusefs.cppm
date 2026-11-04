module;
#include <cstdint>

export module pbsd.fs.fusefs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fuse/fuse.h — FUSE protocol version and init flags.
export namespace pbsd::fs::fusefs {

inline constexpr unsigned kMajorVersion = 7;
inline constexpr unsigned kMinorVersion = 31;
inline constexpr unsigned kMaxWrite = 128 * 1024;

enum class InitFlag : unsigned int {
    AsyncRead = 1u << 0,
    PosixLocks = 1u << 1,
    ExportSupport = 1u << 4,
    DontMask = 1u << 6,
    AutoInvalData = 1u << 12,
};

enum class Op : unsigned int {
    Lookup = 1,
    Forget = 2,
    GetAttr = 3,
    SetAttr = 4,
    Read = 5,
    Write = 6,
    Open = 14,
    Release = 18,
    Init = 26,
};

[[nodiscard]] inline Status validate_version(unsigned major, unsigned minor) noexcept {
    if (major != kMajorVersion || minor > kMinorVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_write_size(unsigned size) noexcept {
    if (size == 0 || size > kMaxWrite) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool op_needs_inode(Op op) noexcept {
    switch (op) {
    case Op::Lookup:
    case Op::GetAttr:
    case Op::SetAttr:
    case Op::Read:
    case Op::Write:
    case Op::Open:
    case Op::Release:
        return true;
    default:
        return false;
    }
}

} // namespace pbsd::fs::fusefs
