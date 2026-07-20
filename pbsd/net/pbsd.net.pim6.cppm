module;
#include <cstdint>

export module pbsd.net.pim6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/pim6.h — PIMv2 header validation.
export namespace pbsd::net::pim6 {

inline constexpr unsigned char kVersion = 2;

enum class MsgType : unsigned char {
    Hello      = 0,
    Register   = 1,
    RegStop    = 2,
    JoinPrune  = 3,
    Bootstrap  = 4,
    Assert     = 5,
    Graft      = 6,
    GraftAck   = 7,
    CrpAdv     = 8,
};

struct Header {
    unsigned char ver{};
    unsigned char type{};
    unsigned char rsv{};
    unsigned short checksum{};
};

[[nodiscard]] inline unsigned char pack_type_ver(MsgType type, unsigned char ver) noexcept {
    return static_cast<unsigned char>((static_cast<unsigned char>(type) & 0x0f)
                                      | ((ver & 0x0f) << 4));
}

[[nodiscard]] inline MsgType unpack_type(unsigned char byte) noexcept {
    return static_cast<MsgType>(byte & 0x0f);
}

[[nodiscard]] inline unsigned char unpack_ver(unsigned char byte) noexcept {
    return (byte >> 4) & 0x0f;
}

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.ver != kVersion) {
        return Status::Protocol;
    }
    if (static_cast<unsigned char>(hdr.type) > static_cast<unsigned char>(MsgType::CrpAdv)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_msg_type(MsgType type) noexcept {
    if (static_cast<unsigned char>(type) > static_cast<unsigned char>(MsgType::CrpAdv)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::pim6
