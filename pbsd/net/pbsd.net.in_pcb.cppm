module;
#include <cstdint>

export module pbsd.net.in_pcb;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_pcb.h — INP_* socket control flags.
export namespace pbsd::net::in_pcb {

enum class Flag : unsigned int {
    RecvOpts      = 0x00000001,
    RecvRetOpts   = 0x00000002,
    RecvDstAddr   = 0x00000004,
    HdrIncl       = 0x00000008,
    HighPort      = 0x00000010,
    LowPort       = 0x00000020,
    AnonPort      = 0x00000040,
    RecvIf        = 0x00000080,
    MtuDisc       = 0x00000100,
    RecvTtl       = 0x00000400,
    DontFrag      = 0x00000800,
    BindAny       = 0x00001000,
    InHashList    = 0x00002000,
    RecvTos       = 0x00004000,
    OnesBcast     = 0x02000000,
    Dropped       = 0x04000000,
    SockRef       = 0x08000000,
    BoundFib      = 0x20000000,
};

enum class Flag2 : unsigned int {
    RecvFlowId       = 0x00000100,
    RecvRssBucketId  = 0x00000200,
    RateLimitChanged = 0x00000400,
    OrigDstAddr      = 0x00000800,
    PcpSet           = 0x00020000,
    PcpBit0          = 0x00040000,
    PcpBit1          = 0x00080000,
    PcpBit2          = 0x00100000,
};

inline constexpr unsigned kControlOpts =
    static_cast<unsigned>(Flag::RecvOpts)
    | static_cast<unsigned>(Flag::RecvRetOpts)
    | static_cast<unsigned>(Flag::RecvDstAddr)
    | static_cast<unsigned>(Flag::RecvIf)
    | static_cast<unsigned>(Flag::RecvTtl)
    | static_cast<unsigned>(Flag::RecvTos);

struct FlagEntry {
    Flag        flag{};
    const char* label{};
};

inline constexpr FlagEntry kFlagTable[] = {
    {Flag::RecvOpts, "recvopts"},
    {Flag::RecvRetOpts, "recvretopts"},
    {Flag::RecvDstAddr, "recvdstaddr"},
    {Flag::HdrIncl, "hdrincl"},
    {Flag::HighPort, "highport"},
    {Flag::LowPort, "lowport"},
    {Flag::RecvIf, "recvif"},
    {Flag::MtuDisc, "mtudisc"},
    {Flag::RecvTtl, "recvttl"},
    {Flag::DontFrag, "dontfrag"},
    {Flag::BindAny, "bindany"},
    {Flag::RecvTos, "recvtos"},
};

[[nodiscard]] inline constexpr std::size_t flag_table_size() noexcept {
    return sizeof(kFlagTable) / sizeof(kFlagTable[0]);
}

[[nodiscard]] inline constexpr bool is_control_opt(Flag f) noexcept {
    return (static_cast<unsigned>(f) & kControlOpts) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::HighPort))
        && (flags & static_cast<unsigned>(Flag::LowPort))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned pcp_from_flags2(unsigned flags2) noexcept {
    return (flags2 & (static_cast<unsigned>(Flag2::PcpBit0)
                      | static_cast<unsigned>(Flag2::PcpBit1)
                      | static_cast<unsigned>(Flag2::PcpBit2))) >> 18;
}

} // namespace pbsd::net::in_pcb
