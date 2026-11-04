module;
#include <cstdint>

export module pbsd.fs.p9fs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/p9fs/p9_protocol.h, p9fs.h — Plan 9 file server.
export namespace pbsd::fs::p9fs {

inline constexpr unsigned kMsizeDefault = 8192;
inline constexpr unsigned kVersion9p2000 = 0x00000600;
inline constexpr unsigned kVersion9p2000u = 0x00000601;
inline constexpr unsigned kVersion9p2000l = 0x00000602;

enum class Cmd : unsigned char {
    Tversion = 100,
    Rversion = 101,
    Tauth    = 102,
    Rauth    = 103,
    Tattach  = 104,
    Rattach  = 105,
    Twalk    = 110,
    Rwalk    = 111,
    Topen    = 112,
    Ropen    = 113,
    Tread    = 116,
    Rread    = 117,
    Twrite   = 118,
    Rwrite   = 119,
    Tclunk   = 120,
    Rclunk   = 121,
    Tremove  = 114,
    Rremove  = 115,
    Tstatfs  = 8,
    Rstatfs  = 9,
};

struct Qid {
    unsigned char mode{};
    unsigned version{};
    unsigned long long path{};
};

struct InodeMeta {
    unsigned long long size{};
    unsigned short type{};
    unsigned mode{};
    unsigned short links_count{};
    unsigned long long qid_path{};
};

[[nodiscard]] inline bool qid_is_dir(Qid const& q) noexcept {
    return (q.mode & 0x80) != 0;
}

[[nodiscard]] inline Status validate_version(unsigned version) noexcept {
    if (version != kVersion9p2000 && version != kVersion9p2000u
        && version != kVersion9p2000l) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_msize(unsigned msize) noexcept {
    if (msize < 4096 || msize > 1024 * 1024) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cmd(Cmd cmd) noexcept {
    switch (cmd) {
    case Cmd::Tversion:
    case Cmd::Rversion:
    case Cmd::Tauth:
    case Cmd::Rauth:
    case Cmd::Tattach:
    case Cmd::Rattach:
    case Cmd::Twalk:
    case Cmd::Rwalk:
    case Cmd::Topen:
    case Cmd::Ropen:
    case Cmd::Tread:
    case Cmd::Rread:
    case Cmd::Twrite:
    case Cmd::Rwrite:
    case Cmd::Tclunk:
    case Cmd::Rclunk:
    case Cmd::Tremove:
    case Cmd::Rremove:
    case Cmd::Tstatfs:
    case Cmd::Rstatfs:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::p9fs
