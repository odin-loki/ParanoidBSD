module;
#include <cstdint>

export module pbsd.fs.smbfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/smbfs/smbfs.h — SMBFS mount flags and version.
export namespace pbsd::fs::smbfs {

inline constexpr unsigned kVerMaj = 1;
inline constexpr unsigned kVerMin = 1012;
inline constexpr unsigned kVersion = kVerMaj * 100000 + kVerMin;
inline constexpr unsigned kMaxPathComp = 256;
inline constexpr unsigned kDefaultPort = 445;

enum class MountFlag : unsigned int {
    Soft = 0x0001,
    Intr = 0x0002,
    Strong = 0x0004,
    HaveNls = 0x0008,
    NoLong = 0x0010,
};

enum class ShareType : unsigned char {
    Disk = 0,
    Printer = 1,
    Pipe = 2,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_server(char const* host, unsigned port) noexcept {
    if (host == nullptr || host[0] == '\0') {
        return Status::Invalid;
    }
    if (port == 0 || port > 65535) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_share(char const* share, ShareType type) noexcept {
    if (share == nullptr || share[0] == '\0') {
        return Status::Invalid;
    }
    if (type == ShareType::Disk && share[0] != '/') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::smbfs
