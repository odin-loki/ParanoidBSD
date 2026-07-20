module;
#include <cstdint>

export module pbsd.net.route;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/route.h — RTF_* routing flags and nexthop API.
export namespace pbsd::net::route {

enum class Flag : unsigned int {
    Up          = 0x00000001,
    Gateway     = 0x00000002,
    Host        = 0x00000004,
    Reject      = 0x00000008,
    Dynamic     = 0x00000010,
    Modified    = 0x00000020,
    Done        = 0x00000040,
    Xresolve    = 0x00000200,
    Lldata      = 0x00000400,
    Static      = 0x00000800,
    Blackhole   = 0x00001000,
    Proto2      = 0x00004000,
    Proto1      = 0x00008000,
    Proto3      = 0x00040000,
    FixedMtu    = 0x00080000,
    Pinned      = 0x00100000,
    Local       = 0x00200000,
    Broadcast   = 0x00400000,
    Multicast   = 0x00800000,
    Sticky      = 0x10000000,
};

enum class NhFlag : unsigned int {
    Multipath  = 0x0008,
    Reject     = 0x0010,
    Blackhole  = 0x0020,
    Redirect   = 0x0040,
    Default    = 0x0080,
    Broadcast  = 0x0100,
    Gateway    = 0x0200,
    Host       = 0x0400,
};

inline constexpr unsigned kChangeMask =
    static_cast<unsigned>(Flag::Proto1)
    | static_cast<unsigned>(Flag::Proto2)
    | static_cast<unsigned>(Flag::Proto3)
    | static_cast<unsigned>(Flag::Blackhole)
    | static_cast<unsigned>(Flag::Reject)
    | static_cast<unsigned>(Flag::Static)
    | static_cast<unsigned>(Flag::Sticky);

struct FlagEntry {
    Flag        flag{};
    const char* label{};
    bool        changeable{};
};

inline constexpr FlagEntry kFlagTable[] = {
    {Flag::Up, "up", false},
    {Flag::Gateway, "gateway", false},
    {Flag::Host, "host", false},
    {Flag::Reject, "reject", true},
    {Flag::Blackhole, "blackhole", true},
    {Flag::Static, "static", true},
    {Flag::Local, "local", false},
    {Flag::Broadcast, "broadcast", false},
    {Flag::Multicast, "multicast", false},
    {Flag::Pinned, "pinned", false},
};

[[nodiscard]] inline constexpr std::size_t flag_table_size() noexcept {
    return sizeof(kFlagTable) / sizeof(kFlagTable[0]);
}

[[nodiscard]] inline constexpr bool is_changeable(Flag f) noexcept {
    return (static_cast<unsigned>(f) & kChangeMask) != 0;
}

[[nodiscard]] inline constexpr bool is_usable(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Up)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Reject))
        && (flags & static_cast<unsigned>(Flag::Blackhole))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::route
