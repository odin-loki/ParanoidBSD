module;
#include <cstdint>

export module pbsd.net.sctp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/sctp.h — SCTP chunk types and socket options.
export namespace pbsd::net::sctp {

enum class ChunkType : unsigned char {
    Data        = 0,
    Init        = 1,
    InitAck     = 2,
    Sack        = 3,
    Heartbeat   = 4,
    HeartbeatAck = 5,
    Abort       = 6,
    Shutdown    = 7,
    ShutdownAck = 8,
};

enum class Option : unsigned int {
    RtoInfo  = 0x00000001,
    AssocInfo = 0x00000002,
    InitMsg  = 0x00000003,
    NoDelay  = 0x00000004,
};

struct Header {
    unsigned short src_port{};
    unsigned short dest_port{};
    unsigned v_tag{};
    unsigned checksum{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if (h.src_port == 0 || h.dest_port == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_chunk(ChunkType t) noexcept {
    switch (t) {
    case ChunkType::Data:
    case ChunkType::Init:
    case ChunkType::InitAck:
    case ChunkType::Sack:
    case ChunkType::Abort:
    case ChunkType::Shutdown:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::sctp
