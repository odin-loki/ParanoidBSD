module;
#include <cstdint>

export module pbsd.kernel.ifnet;

export import pbsd.core;

/// Wave 4 — kernel ifnet link/driver state (sys/net/if.h, net/if_var.h).
export namespace pbsd::kernel::ifnet {

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
    Multicast    = 0x8000,
    Cantconfig   = 0x10000,
    Monitor      = 0x40000,
    Dying        = 0x200000,
};

enum class LinkState : int {
    Unknown = 0,
    Down    = 1,
    Up      = 2,
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

struct IfnetStub {
    unsigned int  if_flags{};
    LinkState     if_link_state{LinkState::Unknown};
    std::uint16_t if_dunit{};
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 10;
}

[[nodiscard]] constexpr bool flags_has(unsigned val, Flag f) noexcept {
    return (val & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] constexpr bool is_up(unsigned flags) noexcept {
    return flags_has(flags, Flag::Up);
}

[[nodiscard]] constexpr bool cant_change(Flag f) noexcept {
    return (kCantChange & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] constexpr Status validate_link_transition(LinkState from,
                                                        LinkState to) noexcept {
    if (from == LinkState::Unknown && to == LinkState::Down) {
        return Status::Ok;
    }
    if (from == LinkState::Down && to == LinkState::Up) {
        return Status::Ok;
    }
    if (from == LinkState::Up && to == LinkState::Down) {
        return Status::Ok;
    }
    if (from == to) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr Status validate_ifnet(const IfnetStub& ifp) noexcept {
    if (flags_has(ifp.if_flags, Flag::Dying)) {
        return Status::Invalid;
    }
    if (is_up(ifp.if_flags) && ifp.if_link_state == LinkState::Down) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::ifnet
