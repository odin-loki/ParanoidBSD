module;
#include <cstdint>

export module pbsd.net.mroute;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_mroute.h — IPv4 multicast routing control.
export namespace pbsd::net::mroute {

inline constexpr int kMrtInit         = 100;
inline constexpr int kMrtDone         = 101;
inline constexpr int kMrtAddVif       = 102;
inline constexpr int kMrtDelVif       = 103;
inline constexpr int kMrtAddMfc       = 104;
inline constexpr int kMrtDelMfc       = 105;
inline constexpr int kMrtVersion      = 106;
inline constexpr int kMrtAssert       = 107;
inline constexpr int kMrtApiSupport   = 109;
inline constexpr int kMrtApiConfig    = 110;
inline constexpr int kMrtAddBwUpcall  = 111;
inline constexpr int kMrtDelBwUpcall  = 112;

inline constexpr unsigned kMaxVifs = 32;
inline constexpr unsigned short kAllVifs = 0xFFFF;

using VifBitmap = unsigned long;
using VifIndex  = unsigned short;

[[nodiscard]] inline void vif_set(VifIndex n, VifBitmap& m) noexcept {
    if (n < kMaxVifs) {
        m |= (1ul << n);
    }
}

[[nodiscard]] inline void vif_clear(VifIndex n, VifBitmap& m) noexcept {
    if (n < kMaxVifs) {
        m &= ~(1ul << n);
    }
}

[[nodiscard]] inline bool vif_is_set(VifIndex n, VifBitmap m) noexcept {
    if (n >= kMaxVifs) {
        return false;
    }
    return (m & (1ul << n)) != 0;
}

[[nodiscard]] inline Status validate_vif_index(VifIndex vif) noexcept {
    if (vif >= kMaxVifs && vif != kAllVifs) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_mrt_cmd(int cmd) noexcept {
    switch (cmd) {
    case kMrtInit:
    case kMrtDone:
    case kMrtAddVif:
    case kMrtDelVif:
    case kMrtAddMfc:
    case kMrtDelMfc:
    case kMrtVersion:
    case kMrtAssert:
    case kMrtApiSupport:
    case kMrtApiConfig:
    case kMrtAddBwUpcall:
    case kMrtDelBwUpcall:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

struct VifConfig {
    VifIndex index{};
    unsigned char threshold{};
    unsigned char rate_limit{};
    unsigned mtu{};
};

[[nodiscard]] inline Status validate_vif(VifConfig const& v) noexcept {
    if (validate_vif_index(v.index) != Status::Ok) {
        return Status::Invalid;
    }
    if (v.mtu < 576 || v.mtu > 9000) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct MfcEntry {
    unsigned char origin[4]{};
    unsigned char group[4]{};
    VifBitmap     oifs{};
    unsigned      pkt_count{};
    unsigned      byte_count{};
};

[[nodiscard]] inline Status validate_mfc(MfcEntry const& m) noexcept {
    bool origin_zero = true;
    bool group_zero = true;
    for (unsigned i = 0; i < 4; ++i) {
        if (m.origin[i] != 0) {
            origin_zero = false;
        }
        if (m.group[i] != 0) {
            group_zero = false;
        }
    }
    if (origin_zero || group_zero) {
        return Status::Invalid;
    }
    if (m.oifs == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::mroute
