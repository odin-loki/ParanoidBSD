module;
#include <cstdint>

export module pbsd.kernel.inpcb;

export import pbsd.core;

/// Wave 4 — Internet protocol control block flags (netinet/in_pcb.h).
export namespace pbsd::kernel::inpcb {

inline constexpr unsigned kInpIpv4       = 0x1;
inline constexpr unsigned kInpIpv6       = 0x2;
inline constexpr unsigned kInpIpv6Proto  = 0x4;

inline constexpr unsigned kInpRecvopts    = 0x00000001;
inline constexpr unsigned kInpRecvretopts = 0x00000002;
inline constexpr unsigned kInpRecvdstaddr = 0x00000004;
inline constexpr unsigned kInpHdrincl     = 0x00000008;
inline constexpr unsigned kInpHighport      = 0x00000010;
inline constexpr unsigned kInpLowport       = 0x00000020;
inline constexpr unsigned kInpRecvif        = 0x00000080;
inline constexpr unsigned kInpMtudisc       = 0x00000100;
inline constexpr unsigned kInpRecvdttl      = 0x00000400;
inline constexpr unsigned kInpBindany       = 0x00001000;
inline constexpr unsigned kInpInhashlist    = 0x00002000;
inline constexpr unsigned kInpRecvtos       = 0x00004000;
inline constexpr unsigned kIn6pIpv6V6only   = 0x00008000;
inline constexpr unsigned kInpDropped       = 0x04000000;
inline constexpr unsigned kInpSockref       = 0x08000000;
inline constexpr unsigned kInpBoundfib      = 0x20000000;

inline constexpr unsigned kInpControlopts =
    kInpRecvopts | kInpRecvretopts | kInpRecvdstaddr | kInpRecvif
    | kInpRecvdttl | kInpRecvtos;

struct FlagEntry {
    unsigned    flag{};
    const char* name{};
};

inline constexpr FlagEntry kFlagTable[] = {
    {kInpRecvopts,    "INP_RECVOPTS"},
    {kInpRecvdstaddr, "INP_RECVDSTADDR"},
    {kInpHdrincl,     "INP_HDRINCL"},
    {kInpBindany,     "INP_BINDANY"},
    {kInpInhashlist,  "INP_INHASHLIST"},
    {kIn6pIpv6V6only, "IN6P_IPV6_V6ONLY"},
    {kInpDropped,     "INP_DROPPED"},
    {kInpBoundfib,    "INP_BOUNDFIB"},
};

struct InpcbStub {
    unsigned inp_vflags{};
    unsigned inp_flags{};
    unsigned inp_fibnum{};
    std::uint16_t inp_lport{};
    std::uint16_t inp_fport{};
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kFlagTable) / sizeof(kFlagTable[0]));
}

[[nodiscard]] constexpr bool has_vflag(unsigned vflags, unsigned bit) noexcept {
    return (vflags & bit) != 0;
}

[[nodiscard]] constexpr bool has_flag(unsigned flags, unsigned bit) noexcept {
    return (flags & bit) != 0;
}

[[nodiscard]] constexpr bool is_ipv4(unsigned vflags) noexcept {
    return has_vflag(vflags, kInpIpv4);
}

[[nodiscard]] constexpr bool is_ipv6(unsigned vflags) noexcept {
    return has_vflag(vflags, kInpIpv6);
}

[[nodiscard]] constexpr Status validate_vflags(unsigned vflags) noexcept {
    if (vflags == 0) {
        return Status::Invalid;
    }
    if (has_vflag(vflags, kInpIpv4) && has_vflag(vflags, kInpIpv6)
        && !has_vflag(vflags, kInpIpv6Proto)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_inpcb(const InpcbStub& inp) noexcept {
    if (validate_vflags(inp.inp_vflags) != Status::Ok) {
        return Status::Invalid;
    }
    if (has_flag(inp.inp_flags, kInpDropped)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Result<const char*> flag_name(unsigned flag) noexcept {
    for (const auto& e : kFlagTable) {
        if (e.flag == flag) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

} // namespace pbsd::kernel::inpcb
