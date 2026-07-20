module;
#include <cstdint>

export module pbsd.fs.fcntl;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/fcntl.h — O_* open flags and F_* fcntl commands.
export namespace pbsd::fs::fcntl {

enum class OpenFlag : unsigned int {
    Rdonly   = 0x0000,
    Wronly   = 0x0001,
    Rdwr     = 0x0002,
    Nonblock = 0x0004,
    Append   = 0x0008,
    Shlock   = 0x0010,
    Exlock   = 0x0020,
    Async    = 0x0040,
    Sync     = 0x0080,
    NoFollow = 0x0100,
    Creat    = 0x0200,
    Trunc    = 0x0400,
    Excl     = 0x0800,
    Direct   = 0x00010000,
    Directory = 0x00020000,
};

enum class FcntlCmd : unsigned int {
    DupFd       = 0,
    GetFd       = 1,
    SetFd       = 2,
    GetFlags    = 3,
    SetFlags    = 4,
    GetOwn      = 5,
    SetOwn      = 6,
    GetLk       = 7,
    SetLk       = 8,
    SetLkw      = 9,
    GetOwnEx    = 16,
    SetOwnEx    = 17,
};

[[nodiscard]] inline CapabilityRights rights_for(OpenFlag f) noexcept {
    switch (f) {
    case OpenFlag::Rdonly:
        return CapabilityRights::Read;
    case OpenFlag::Wronly:
    case OpenFlag::Append:
    case OpenFlag::Trunc:
    case OpenFlag::Creat:
        return CapabilityRights::Write;
    case OpenFlag::Rdwr:
        return CapabilityRights::Read | CapabilityRights::Write;
    default:
        return CapabilityRights::None;
    }
}

[[nodiscard]] inline Status check_open(CapabilityRights held, OpenFlag f) noexcept {
    CapabilityRights need = rights_for(f);
    return has_right(held, need) ? Status::Ok : Status::Denied;
}

} // namespace pbsd::fs::fcntl
