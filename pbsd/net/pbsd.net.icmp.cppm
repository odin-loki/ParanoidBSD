module;
#include <cstdint>

export module pbsd.net.icmp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_icmp.h — ICMP type/code tables.
export namespace pbsd::net::icmp {

inline constexpr unsigned char kMinLen = 8;

enum class Type : unsigned char {
    EchoReply       = 0,
    Unreach         = 3,
    SourceQuench    = 4,
    Redirect        = 5,
    AltHostAddr     = 6,
    Echo            = 8,
    RouterAdvert    = 9,
    RouterSolicit   = 10,
    Timxceed        = 11,
    ParamProb       = 12,
    Tstamp          = 13,
    TstampReply     = 14,
    IreQ            = 15,
    IreQReply       = 16,
    MaskReq         = 17,
    MaskReply       = 18,
    Traceroute      = 30,
    Photuris        = 40,
    MaxType         = 40,
};

enum class UnreachCode : unsigned char {
    Net              = 0,
    Host             = 1,
    Protocol         = 2,
    Port             = 3,
    NeedFrag         = 4,
    SrcFail          = 5,
    NetUnknown       = 6,
    HostUnknown      = 7,
    Isolated         = 8,
    NetProhib        = 9,
    HostProhib       = 10,
    TosNet           = 11,
    TosHost          = 12,
    FilterProhib     = 13,
    HostPrecedence   = 14,
    PrecedenceCutoff = 15,
};

enum class RedirectCode : unsigned char {
    Net     = 0,
    Host    = 1,
    TosNet  = 2,
    TosHost = 3,
};

enum class TimxceedCode : unsigned char {
    InTrans = 0,
    Reass   = 1,
};

struct Header {
    unsigned char type{};
    unsigned char code{};
    unsigned short checksum{};
    unsigned short id{};
    unsigned short seq{};
};

struct TypeEntry {
    Type        type{};
    const char* label{};
    bool        is_info{};
};

inline constexpr TypeEntry kTypeTable[] = {
    {Type::EchoReply, "echoreply", true},
    {Type::Unreach, "unreach", false},
    {Type::Redirect, "redirect", false},
    {Type::Echo, "echo", true},
    {Type::RouterAdvert, "routeradvert", true},
    {Type::RouterSolicit, "routersolicit", true},
    {Type::Timxceed, "timxceed", false},
    {Type::ParamProb, "paramprob", false},
    {Type::Tstamp, "tstamp", true},
    {Type::TstampReply, "tstampreply", true},
    {Type::MaskReq, "maskreq", true},
    {Type::MaskReply, "maskreply", true},
};

[[nodiscard]] inline constexpr std::size_t type_table_size() noexcept {
    return sizeof(kTypeTable) / sizeof(kTypeTable[0]);
}

[[nodiscard]] inline constexpr bool is_info_type(Type t) noexcept {
    switch (t) {
    case Type::EchoReply:
    case Type::Echo:
    case Type::RouterAdvert:
    case Type::RouterSolicit:
    case Type::Tstamp:
    case Type::TstampReply:
    case Type::IreQ:
    case Type::IreQReply:
    case Type::MaskReq:
    case Type::MaskReply:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline constexpr bool is_error_type(Type t) noexcept {
    return !is_info_type(t) && t != Type::AltHostAddr && t != Type::Traceroute;
}

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.type > static_cast<unsigned char>(Type::MaxType)) {
        return Status::Protocol;
    }
    if (hdr.type == static_cast<unsigned char>(Type::Unreach)
        && hdr.code > static_cast<unsigned char>(UnreachCode::PrecedenceCutoff)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::icmp
