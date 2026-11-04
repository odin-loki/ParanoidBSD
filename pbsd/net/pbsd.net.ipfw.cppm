module;
#include <cstdint>

export module pbsd.net.ipfw;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_fw.h — ipfw(4) rule and set limits.
export namespace pbsd::net::ipfw {

inline constexpr unsigned kDefaultRule = 65535;
inline constexpr unsigned kMaxSets = 32;
inline constexpr unsigned kResvdSet = 31;
inline constexpr unsigned kArgMin = 1;
inline constexpr unsigned kArgMax = 65534;
inline constexpr unsigned kCallstackSize = 16;

enum class OpVer : unsigned short {
    V0 = 0,
    V1 = 1,
};

enum class ArgFlag : unsigned int {
    Ether  = 0x00010000,
    Nh4    = 0x00020000,
    Nh6    = 0x00040000,
    In     = 0x00400000,
    Out    = 0x00800000,
    Ip4    = 0x01000000,
    Ip6    = 0x02000000,
};

[[nodiscard]] inline Status validate_rule_num(unsigned rule) noexcept {
    if (rule > kDefaultRule) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_set(unsigned set) noexcept {
    if (set >= kMaxSets) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ipfw
