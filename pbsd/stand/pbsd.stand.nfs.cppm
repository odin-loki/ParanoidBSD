module;
#include <cstdint>

export module pbsd.stand.nfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/nfsv2.h, libsa/nfs.c
export namespace pbsd::stand::nfs {

inline constexpr unsigned kPort = 2049;
inline constexpr unsigned kProg = 100003;
inline constexpr unsigned kVer2 = 2;
inline constexpr unsigned kMaxData = 32768;
inline constexpr unsigned kMaxPathLen = 1024;
inline constexpr unsigned kMaxNameLen = 255;
inline constexpr unsigned kFhSize = 32;
inline constexpr unsigned kMinPacket = 20;
inline constexpr unsigned kNprocs = 18;

enum class RpcStatus : unsigned char {
    Ok = 0,
    ErrPerm = 1,
    ErrNoent = 2,
    ErrIo = 5,
    ErrNxmio = 63,
};

[[nodiscard]] inline Status validate_fh_len(unsigned len) noexcept {
    return len <= kFhSize ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_path_len(unsigned len) noexcept {
    return len > 0 && len <= kMaxPathLen ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::nfs
