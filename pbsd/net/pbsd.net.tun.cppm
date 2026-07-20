module;
#include <cstdint>

export module pbsd.net.tun;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_tun.h — TUN device MTU and ioctls.
export namespace pbsd::net::tun {

inline constexpr unsigned kMtuDefault = 1500;
inline constexpr unsigned kMruMax = 65535;

struct Info {
    int baudrate{};
    unsigned short mtu{};
    unsigned char type{};
};

[[nodiscard]] inline Status validate_mtu(unsigned mtu) noexcept {
    if (mtu == 0 || mtu > kMruMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_info(Info const& info) noexcept {
    return validate_mtu(info.mtu);
}

} // namespace pbsd::net::tun
