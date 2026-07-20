module;
#include <cstdint>

export module pbsd.net.bridge_fdb;

import pbsd.core;
import pbsd.net.bridge;

/// PROVENANCE: hbsd/src/sys/net/if_bridge.c, if_bridgevar.h — bridge FDB and ioctl.
export namespace pbsd::net::bridge::fdb {

inline constexpr unsigned kHashSize = 1024;
inline constexpr unsigned kHashMask = kHashSize - 1;
inline constexpr unsigned kMaxEntries = 2000;
inline constexpr unsigned kTimeoutSec = 20 * 60;
inline constexpr unsigned kPrunePeriodSec = 5 * 60;

enum class Ioctl : unsigned char {
    Add = 0,
    Del = 1,
    GetFlags = 2,
    SetFlags = 3,
    SetCache = 4,
    GetCache = 5,
    GetMembers = 6,
    GetAddrs = 7,
    SetStaticAddr = 8,
    SetTimeout = 9,
    GetTimeout = 10,
    DelAddr = 11,
    Flush = 12,
};

struct MacAddr {
    unsigned char bytes[6]{};
};

struct Entry {
    MacAddr dst{};
    unsigned char portno{};
    unsigned age_sec{};
    bool static_entry{};
};

[[nodiscard]] inline unsigned hash_mac(MacAddr const& mac) noexcept {
    unsigned h = mac.bytes[0] ^ mac.bytes[1] ^ mac.bytes[2];
    h ^= mac.bytes[3] ^ mac.bytes[4] ^ mac.bytes[5];
    return h & kHashMask;
}

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    bool all_zero = true;
    for (unsigned char b : e.dst.bytes) {
        if (b != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        return Status::Invalid;
    }
    if (e.portno == 0 && !e.static_entry) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cache_size(unsigned size) noexcept {
    if (size == 0 || size > kMaxEntries) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_ioctl(Ioctl cmd, unsigned flags) noexcept {
    if (cmd == Ioctl::SetFlags || cmd == Ioctl::GetFlags) {
        return bridge::validate_flags(flags);
    }
    return Status::Ok;
}

} // namespace pbsd::net::bridge::fdb
