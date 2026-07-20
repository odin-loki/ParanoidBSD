module;
#include <cstdint>

export module pbsd.kernel.ether;

import pbsd.core;

/// Freestanding port of `net/if_ethersubr.c` — Ethernet address helpers.
export namespace pbsd::kernel::ether {

inline constexpr unsigned kAddrLen = 6u;
inline constexpr unsigned kHdrLen  = 14u;
inline constexpr unsigned kCrcLen  = 4u;
inline constexpr unsigned kMinFrame = 64u;

using MacAddr = unsigned char[kAddrLen];

inline constexpr MacAddr kBroadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

[[nodiscard]] inline bool is_broadcast(const MacAddr& ea) noexcept {
    for (unsigned i = 0; i < kAddrLen; ++i) {
        if (ea[i] != 0xff) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_multicast(const MacAddr& ea) noexcept {
    return (ea[0] & 0x01) != 0;
}

[[nodiscard]] inline bool is_zero(const MacAddr& ea) noexcept {
    for (unsigned i = 0; i < kAddrLen; ++i) {
        if (ea[i] != 0) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline Status validate_addr(const MacAddr& ea) noexcept {
    if (is_zero(ea) || is_multicast(ea)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned frame_len(unsigned payload) noexcept {
    const unsigned len = kHdrLen + payload + kCrcLen;
    return len < kMinFrame ? kMinFrame : len;
}

} // namespace pbsd::kernel::ether
