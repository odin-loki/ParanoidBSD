module;
#include <cstdint>

export module pbsd.net.arp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_arp.h — ARP hardware/opcodes.
export namespace pbsd::net::arp {

enum class Hardware : unsigned short {
    Ether       = 1,
    Ieee802     = 6,
    FrameRelay  = 15,
    Ieee1394    = 24,
    Infiniband  = 32,
};

enum class Op : unsigned short {
    Request    = 1,
    Reply      = 2,
    RevRequest = 3,
    RevReply   = 4,
    InvRequest = 8,
    InvReply   = 9,
};

struct Header {
    unsigned short hw_type{};
    unsigned short proto_type{};
    unsigned char  hw_len{};
    unsigned char  proto_len{};
    unsigned short op{};
};

struct OpEntry {
    Op          op{};
    const char* label{};
    bool        is_request{};
};

inline constexpr OpEntry kOpTable[] = {
    {Op::Request, "request", true},
    {Op::Reply, "reply", false},
    {Op::RevRequest, "revrequest", true},
    {Op::RevReply, "revreply", false},
    {Op::InvRequest, "invrequest", true},
    {Op::InvReply, "invreply", false},
};

[[nodiscard]] inline constexpr std::size_t op_table_size() noexcept {
    return sizeof(kOpTable) / sizeof(kOpTable[0]);
}

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.hw_len == 0 || hdr.proto_len == 0) {
        return Status::Protocol;
    }
    if (hdr.hw_type != static_cast<unsigned short>(Hardware::Ether)
        && hdr.hw_type != static_cast<unsigned short>(Hardware::Infiniband)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::arp
