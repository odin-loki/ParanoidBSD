module;
#include <cstddef>
#include <cstdint>

export module pbsd.net.icmp6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/icmp6.h — ICMPv6 type/code tables.
export namespace pbsd::net::icmp6 {

inline constexpr unsigned kPldMaxLen = 1232;

enum class Type : unsigned char {
    DstUnreach      = 1,
    PacketTooBig    = 2,
    TimeExceeded    = 3,
    ParamProb       = 4,
    EchoRequest     = 128,
    EchoReply       = 129,
    MldQuery        = 130,
    MldReport       = 131,
    MldDone         = 132,
    RouterSolicit   = 133,
    RouterAdvert    = 134,
    NeighborSolicit = 135,
    NeighborAdvert  = 136,
    Redirect        = 137,
    RouterRenumber  = 138,
    Mldv2Report     = 143,
    MaxType         = 201,
};

enum class DstUnreachCode : unsigned char {
    NoRoute         = 0,
    AdminProhibited = 1,
    BeyondScope     = 2,
    AddrUnreach     = 3,
    PortUnreach     = 4,
    PolicyFail      = 5,
    RejectRoute     = 6,
    SrcRouteError   = 7,
};

struct Header {
    unsigned char type{};
    unsigned char code{};
    unsigned short checksum{};
    unsigned id{};
    unsigned seq{};
};

struct TypeEntry {
    Type        type{};
    const char* label{};
    bool        is_info{};
};

inline constexpr TypeEntry kTypeTable[] = {
    {Type::DstUnreach,      "dst_unreach",      false},
    {Type::PacketTooBig,    "packet_too_big",   false},
    {Type::TimeExceeded,    "time_exceeded",    false},
    {Type::ParamProb,       "param_prob",       false},
    {Type::EchoRequest,     "echo_request",     true},
    {Type::EchoReply,       "echo_reply",       true},
    {Type::MldQuery,        "mld_query",        true},
    {Type::MldReport,       "mld_report",       true},
    {Type::MldDone,         "mld_done",         true},
    {Type::RouterSolicit,   "router_solicit",   true},
    {Type::RouterAdvert,    "router_advert",    true},
    {Type::NeighborSolicit, "neighbor_solicit", true},
    {Type::NeighborAdvert,  "neighbor_advert",  true},
    {Type::Redirect,        "redirect",         true},
    {Type::Mldv2Report,     "mldv2_report",     true},
};

[[nodiscard]] inline std::size_t type_table_size() noexcept {
    return sizeof(kTypeTable) / sizeof(kTypeTable[0]);
}

[[nodiscard]] inline bool is_info_type(Type t) noexcept {
    return static_cast<unsigned char>(t) >= 128;
}

[[nodiscard]] inline bool is_error_type(Type t) noexcept {
    return static_cast<unsigned char>(t) < 128 && static_cast<unsigned char>(t) != 0;
}

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (static_cast<unsigned char>(hdr.type) > static_cast<unsigned char>(Type::MaxType)) {
        return Status::Invalid;
    }
    if (is_error_type(static_cast<Type>(hdr.type)) && hdr.code > 15) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_payload_len(unsigned len) noexcept {
    if (len > kPldMaxLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::icmp6
