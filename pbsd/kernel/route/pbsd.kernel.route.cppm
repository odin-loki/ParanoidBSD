module;
#include <cstdint>

export module pbsd.kernel.route;

export import pbsd.core;

/// Wave 4 — kernel routing flags (sys/net/route.h, net/route.c).
export namespace pbsd::kernel::route {

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

inline constexpr unsigned kChangeMask =
    static_cast<unsigned>(Flag::Proto1)
    | static_cast<unsigned>(Flag::Proto2)
    | static_cast<unsigned>(Flag::Proto3)
    | static_cast<unsigned>(Flag::Blackhole)
    | static_cast<unsigned>(Flag::Reject)
    | static_cast<unsigned>(Flag::Static)
    | static_cast<unsigned>(Flag::Sticky);

struct RouteStub {
    unsigned rtf_flags{};
    unsigned fibnum{};
    std::uint16_t rt_weight{};
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 12;
}

[[nodiscard]] constexpr bool is_changeable(Flag f) noexcept {
    return (static_cast<unsigned>(f) & kChangeMask) != 0;
}

[[nodiscard]] constexpr bool is_usable(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Up)) != 0;
}

[[nodiscard]] constexpr Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Reject))
        && (flags & static_cast<unsigned>(Flag::Blackhole))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_route(const RouteStub& rt) noexcept {
    if (validate_flags(rt.rtf_flags) != Status::Ok) {
        return Status::Invalid;
    }
    if (!is_usable(rt.rtf_flags)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::route
