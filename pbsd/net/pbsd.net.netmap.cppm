module;
#include <cstdint>

export module pbsd.net.netmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/netmap.h — netmap API version and ring flags.
export namespace pbsd::net::netmap {

inline constexpr unsigned kApi = 14;
inline constexpr unsigned kMinApi = 14;
inline constexpr unsigned kMaxApi = 15;
inline constexpr unsigned kCacheAlign = 128;

enum class RingFlag : unsigned int {
    TxRing = 0x0001,
    RxRing = 0x0002,
    HostRing = 0x0004,
};

[[nodiscard]] inline Status validate_api(unsigned api) noexcept {
    if (api < kMinApi || api > kMaxApi) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::netmap
