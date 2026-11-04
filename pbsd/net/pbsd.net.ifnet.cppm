module;
#include <cstdint>

export module pbsd.net.ifnet;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/net/if.h — interface flags and capability masks.
export namespace pbsd::net::ifnet {

enum class Flag : unsigned int {
    Up           = 0x0001,
    Broadcast    = 0x0002,
    Debug        = 0x0004,
    Loopback     = 0x0008,
    Pointopoint  = 0x0010,
    Needsepoch   = 0x0020,
    DrvRunning   = 0x0040,
    Noarp        = 0x0080,
    Promisc      = 0x0100,
    Allmulti     = 0x0200,
    DrvOactive   = 0x0400,
    Simplex      = 0x0800,
    Link0        = 0x1000,
    Link1        = 0x2000,
    Link2        = 0x4000,
    Multicast    = 0x8000,
    Cantconfig   = 0x10000,
    Ppromisc     = 0x20000,
    Monitor      = 0x40000,
    Staticarp    = 0x80000,
    Stickyarp    = 0x100000,
    Dying        = 0x200000,
    Renaming     = 0x400000,
    Pallmulti    = 0x800000,
    Netlink1     = 0x1000000,
};

inline constexpr unsigned kCantChange =
    static_cast<unsigned>(Flag::Broadcast)
    | static_cast<unsigned>(Flag::Pointopoint)
    | static_cast<unsigned>(Flag::DrvRunning)
    | static_cast<unsigned>(Flag::DrvOactive)
    | static_cast<unsigned>(Flag::Simplex)
    | static_cast<unsigned>(Flag::Multicast)
    | static_cast<unsigned>(Flag::Allmulti)
    | static_cast<unsigned>(Flag::Promisc)
    | static_cast<unsigned>(Flag::Dying)
    | static_cast<unsigned>(Flag::Cantconfig)
    | static_cast<unsigned>(Flag::Needsepoch);

struct FlagEntry {
    Flag            flag{};
    const char*     name{};
    CapabilityRights need_read{CapabilityRights::None};
    CapabilityRights need_write{CapabilityRights::None};
};

inline constexpr FlagEntry kIfFlagTable[] = {
    {Flag::Up, "up", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Debug, "debug", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Promisc, "promisc", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Allmulti, "allmulti", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Monitor, "monitor", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Noarp, "noarp", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Staticarp, "staticarp", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Loopback, "loopback", CapabilityRights::Read, CapabilityRights::None},
    {Flag::Broadcast, "broadcast", CapabilityRights::Read, CapabilityRights::None},
    {Flag::Pointopoint, "pointopoint", CapabilityRights::Read, CapabilityRights::None},
    {Flag::Multicast, "multicast", CapabilityRights::Read, CapabilityRights::None},
};

[[nodiscard]] inline constexpr std::size_t if_flag_table_size() noexcept {
    return sizeof(kIfFlagTable) / sizeof(kIfFlagTable[0]);
}

[[nodiscard]] inline constexpr bool flags_has(unsigned int val, Flag f) noexcept {
    return (val & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] inline constexpr bool is_up(unsigned int flags) noexcept {
    return flags_has(flags, Flag::Up);
}

[[nodiscard]] inline constexpr bool cant_change(Flag f) noexcept {
    return (kCantChange & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] inline Status check_flag_change(CapabilityRights if_rights, Flag f,
                                              bool set) noexcept {
    for (auto const& e : kIfFlagTable) {
        if (e.flag != f) {
            continue;
        }
        if (!has_right(if_rights, e.need_read)) {
            return Status::Denied;
        }
        if (set && e.need_write != CapabilityRights::None
            && !has_right(if_rights, e.need_write)) {
            return Status::Denied;
        }
        return Status::Ok;
    }
    if (cant_change(f)) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ifnet
