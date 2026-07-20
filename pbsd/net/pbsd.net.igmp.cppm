module;
#include <cstdint>

export module pbsd.net.igmp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/igmp.h — IGMP message types.
export namespace pbsd::net::igmp {

inline constexpr unsigned kMinLen = 8;
inline constexpr unsigned kV3QueryMinLen = 12;

enum class Type : unsigned char {
    HostMembershipQuery     = 0x11,
    V1HostMembershipReport  = 0x12,
    Dvmrp                   = 0x13,
    Pim                     = 0x14,
    V2HostMembershipReport  = 0x16,
    HostLeaveMessage        = 0x17,
    MtraceReply             = 0x1e,
    MtraceQuery             = 0x1f,
    V3HostMembershipReport  = 0x22,
};

enum class FilterMode : unsigned char {
    DoNothing      = 0,
    ModeIsInclude  = 1,
    ModeIsExclude  = 2,
};

struct Header {
    unsigned char  type{};
    unsigned char  code{};
    unsigned short cksum{};
    unsigned       group{};
};

[[nodiscard]] inline constexpr std::size_t type_table_size() noexcept { return 9; }

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.type == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::igmp
