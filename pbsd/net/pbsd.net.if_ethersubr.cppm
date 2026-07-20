module;
#include <cstdint>

export module pbsd.net.if_ethersubr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_ethersubr.c — Ethernet framing helpers.
export namespace pbsd::net::if_ethersubr {

inline constexpr unsigned kAddrLen = 6;
inline constexpr unsigned kTypeLen = 2;
inline constexpr unsigned kHdrLen = 14;
inline constexpr unsigned kMinPayload = 46;
inline constexpr unsigned kMaxPayload = 1500;
inline constexpr unsigned kCrcLen = 4;

enum class EtherType : std::uint16_t {
    Ip = 0x0800,
    Arp = 0x0806,
    Ipv6 = 0x86DD,
    Vlan = 0x8100,
};

[[nodiscard]] inline Status validate_frame_len(unsigned total) noexcept {
    if (total < kHdrLen + kMinPayload) {
        return Status::Invalid;
    }
    if (total > kHdrLen + kMaxPayload + kCrcLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_multicast(const std::uint8_t addr[kAddrLen]) noexcept {
    return addr != nullptr && (addr[0] & 0x01u) != 0;
}

[[nodiscard]] inline bool is_broadcast(const std::uint8_t addr[kAddrLen]) noexcept {
    if (addr == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < kAddrLen; ++i) {
        if (addr[i] != 0xffu) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::net::if_ethersubr
