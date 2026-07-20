module;
#include <cstdint>

export module pbsd.net.netinet;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in.h — protocol numbers and IP-level options.
export namespace pbsd::net::netinet {

enum class IpProto : int {
    Ip       = 0,
    HopOpts  = 0,
    Icmp     = 1,
    Igmp     = 2,
    Ipv4     = 4,
    Tcp      = 6,
    Egp      = 8,
    Pup      = 12,
    Udp      = 17,
    Idp      = 22,
    Tp       = 29,
    Dccp     = 33,
    Ipv6     = 41,
    Routing  = 43,
    Fragment = 44,
    Gre      = 47,
    Esp      = 50,
    Ah       = 51,
    Icmpv6   = 58,
    None     = 59,
    Dstopts  = 60,
    Pim      = 103,
    Comp     = 108,
    Raw      = 255,
};

struct IpProtoEntry {
    IpProto     proto{};
    const char* name{};
};

inline constexpr IpProtoEntry kIpProtoTable[] = {
    {IpProto::Icmp, "icmp"},
    {IpProto::Igmp, "igmp"},
    {IpProto::Tcp, "tcp"},
    {IpProto::Udp, "udp"},
    {IpProto::Gre, "gre"},
    {IpProto::Esp, "esp"},
    {IpProto::Ah, "ah"},
    {IpProto::Ipv6, "ipv6"},
    {IpProto::Icmpv6, "icmpv6"},
    {IpProto::Pim, "pim"},
    {IpProto::Raw, "raw"},
};

enum class IpOption : int {
    Options        = 1,
    HdrIncl        = 2,
    Tos            = 3,
    Ttl            = 4,
    RecvOpts       = 5,
    RecvRetOpts    = 6,
    RecvDstAddr    = 7,
    RetOpts        = 8,
    MulticastIf    = 9,
    MulticastTtl   = 10,
    MulticastLoop  = 11,
    AddMembership  = 12,
    DropMembership = 13,
    PortRange      = 19,
    RecvIf         = 20,
    IpsecPolicy    = 21,
    BindAny        = 24,
    OrigDstAddr    = 27,
};

inline constexpr std::uint32_t kInaddrAny       = 0x00000000u;
inline constexpr std::uint32_t kInaddrBroadcast = 0xffffffffu;

inline constexpr std::uint16_t kIpPortReserved = 1024;
inline constexpr std::uint16_t kIpPortUser     = 5000;

struct IpOptionEntry {
    IpOption name{};
    const char* label{};
};

inline constexpr IpOptionEntry kIpOptionTable[] = {
    {IpOption::Ttl, "ip_ttl"},
    {IpOption::Tos, "ip_tos"},
    {IpOption::HdrIncl, "ip_hdrincl"},
    {IpOption::MulticastTtl, "ip_multicast_ttl"},
    {IpOption::MulticastLoop, "ip_multicast_loop"},
    {IpOption::AddMembership, "ip_add_membership"},
    {IpOption::DropMembership, "ip_drop_membership"},
    {IpOption::BindAny, "ip_bindany"},
    {IpOption::OrigDstAddr, "ip_origdstaddr"},
};

[[nodiscard]] inline constexpr std::size_t ip_option_table_size() noexcept {
    return sizeof(kIpOptionTable) / sizeof(kIpOptionTable[0]);
}

[[nodiscard]] inline constexpr std::size_t ip_proto_table_size() noexcept {
    return sizeof(kIpProtoTable) / sizeof(kIpProtoTable[0]);
}

[[nodiscard]] inline constexpr bool is_transport_proto(IpProto p) noexcept {
    return p == IpProto::Tcp || p == IpProto::Udp || p == IpProto::Dccp;
}

[[nodiscard]] inline constexpr bool is_multicast_option(IpOption opt) noexcept {
    switch (opt) {
    case IpOption::MulticastIf:
    case IpOption::MulticastTtl:
    case IpOption::MulticastLoop:
    case IpOption::AddMembership:
    case IpOption::DropMembership:
        return true;
    default:
        return false;
    }
}

} // namespace pbsd::net::netinet
