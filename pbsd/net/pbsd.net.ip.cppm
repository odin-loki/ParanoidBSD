module;
#include <cstdint>

export module pbsd.net.ip;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip.h — IPv4 header layout and TOS/DSCP tables.
export namespace pbsd::net::ip {

inline constexpr unsigned char kIpVersion = 4;
inline constexpr unsigned short kMaxPacket = 65535;

enum class FragmentFlag : unsigned short {
    Reserved = 0x8000, // IP_RF
    DontFrag = 0x4000, // IP_DF
    MoreFrag = 0x2000, // IP_MF
    OffMask  = 0x1fff, // IP_OFFMASK
};

enum class Tos : unsigned char {
    LowDelay    = 0x10, // IPTOS_LOWDELAY
    Throughput  = 0x08, // IPTOS_THROUGHPUT
    Reliability = 0x04, // IPTOS_RELIABILITY
    MinCost     = 0x00, // IPTOS_MINCOST / DSCP CS0
};

enum class Dscp : unsigned char {
    Cs0  = 0x00,
    Cs1  = 0x08,
    Cs2  = 0x10,
    Cs3  = 0x18,
    Cs4  = 0x20,
    Cs5  = 0x28,
    Cs6  = 0x30,
    Cs7  = 0x38,
    Ef   = 0x2e, // expedited forwarding
    Af11 = 0x0a,
    Af12 = 0x0c,
    Af13 = 0x0e,
    Af21 = 0x12,
    Af22 = 0x14,
    Af23 = 0x16,
    Af31 = 0x1a,
    Af32 = 0x1c,
    Af33 = 0x1e,
    Af41 = 0x22,
    Af42 = 0x24,
    Af43 = 0x26,
};

struct IpHeader {
    unsigned char  version_hl{};
    unsigned char  tos{};
    unsigned short total_len{};
    unsigned short id{};
    unsigned short offset{};
    unsigned char  ttl{};
    unsigned char  protocol{};
    unsigned short checksum{};
    std::uint32_t  src{};
    std::uint32_t  dst{};
};

struct TosEntry {
    Tos         tag{};
    const char* label{};
};

inline constexpr TosEntry kTosTable[] = {
    {Tos::LowDelay, "lowdelay"},
    {Tos::Throughput, "throughput"},
    {Tos::Reliability, "reliability"},
    {Tos::MinCost, "mincost"},
};

[[nodiscard]] inline constexpr std::size_t tos_table_size() noexcept {
    return sizeof(kTosTable) / sizeof(kTosTable[0]);
}

[[nodiscard]] inline constexpr unsigned char dscp_to_tos(Dscp d) noexcept {
    return static_cast<unsigned char>(d) << 2;
}

[[nodiscard]] inline constexpr bool has_fragment_flag(unsigned short off,
                                                      FragmentFlag flag) noexcept {
    return (off & static_cast<unsigned short>(flag)) != 0;
}

[[nodiscard]] inline Status validate_header(IpHeader const& hdr) noexcept {
    unsigned ver = (hdr.version_hl >> 4) & 0x0f;
    if (ver != kIpVersion) {
        return Status::Protocol;
    }
    if (hdr.total_len < 20 || hdr.total_len > kMaxPacket) {
        return Status::Invalid;
    }
    if (hdr.ttl == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip
