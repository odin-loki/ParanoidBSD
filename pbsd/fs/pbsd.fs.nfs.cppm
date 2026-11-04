module;
#include <cstdint>

export module pbsd.fs.nfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nfs/nfsproto.h — NFS protocol constants.
export namespace pbsd::fs::nfs {

inline constexpr unsigned kPort = 2049;
inline constexpr unsigned kProg = 100003;
inline constexpr unsigned kVer2 = 2;
inline constexpr unsigned kVer3 = 3;
inline constexpr unsigned kVer4 = 4;
inline constexpr unsigned kMaxPathLen = 1024;
inline constexpr unsigned kMaxNameLen = 255;

enum class Proc : unsigned short {
    Null = 0,
    GetAttr = 1,
    Lookup = 3,
    Read = 6,
    Write = 8,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver < kVer2 || ver > kVer4) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::nfs
