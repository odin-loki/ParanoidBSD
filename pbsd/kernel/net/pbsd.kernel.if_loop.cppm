module;
#include <cstdint>

export module pbsd.kernel.if_loop;

import pbsd.core;

/// Freestanding port of `net/if_loop.c` loopback interface constants.
export namespace pbsd::kernel::if_loop {

inline constexpr unsigned kDefaultMtu  = 16384;
inline constexpr unsigned kLargeMtu    = 131072;
inline constexpr unsigned kTinyMtu     = 1536;
inline constexpr const char kName[]    = "lo";

inline constexpr unsigned kCsumIp      = 1u << 0;
inline constexpr unsigned kCsumTcp     = 1u << 1;
inline constexpr unsigned kCsumUdp     = 1u << 2;
inline constexpr unsigned kCsumSctp    = 1u << 3;
inline constexpr unsigned kCsumTcpV6   = 1u << 4;
inline constexpr unsigned kCsumUdpV6   = 1u << 5;
inline constexpr unsigned kCsumSctpV6  = 1u << 6;

inline constexpr unsigned kCsumFeatures =
    kCsumIp | kCsumTcp | kCsumUdp | kCsumSctp;
inline constexpr unsigned kCsumFeatures6 =
    kCsumTcpV6 | kCsumUdpV6 | kCsumSctpV6;

struct Softc {
    bool attached{};
    unsigned mtu{kDefaultMtu};
    unsigned csum_offload{kCsumFeatures | kCsumFeatures6};
};

[[nodiscard]] inline Status attach(Softc& sc, unsigned mtu) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    if (mtu < 68 || mtu > kLargeMtu) {
        return Status::Invalid;
    }
    sc.mtu = mtu;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status detach(Softc& sc) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.attached = false;
    return Status::Ok;
}

[[nodiscard]] inline Status output(Softc& sc, unsigned len) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    if (len > sc.mtu) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::if_loop
