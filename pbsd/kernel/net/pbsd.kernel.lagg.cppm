export module pbsd.kernel.lagg;

export import pbsd.core;

/// Wave 4 — link aggregation (net/if_lagg.h).
export namespace pbsd::kernel::lagg {

inline constexpr unsigned kMaxPorts = 32;
inline constexpr unsigned kMaxNameSize = 32;
inline constexpr unsigned kMaxStacking = 4;

enum class Flag : unsigned {
    HashL2 = 0x00000001,
    HashL3 = 0x00000002,
    HashL4 = 0x00000004,
};

enum class PortFlag : unsigned {
    Slave = 0x00000000,
    Master = 0x00000001,
    Stack = 0x00000002,
    Active = 0x00000004,
    Collecting = 0x00000008,
    Distributing = 0x00000010,
};

enum class Proto : unsigned {
    None = 0,
    RoundRobin = 1,
    Failover = 2,
    LoadBalance = 3,
    Lacp = 4,
    Broadcast = 5,
};

struct PortStub {
    unsigned flags{};
    unsigned weight{};
};

[[nodiscard]] constexpr Flag operator|(Flag a, Flag b) noexcept {
    return static_cast<Flag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool port_active(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(PortFlag::Active)) != 0;
}

[[nodiscard]] constexpr Status validate_proto(Proto p) noexcept {
    if (static_cast<unsigned>(p) >= static_cast<unsigned>(Proto::Broadcast) + 1) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_port(const PortStub& port) noexcept {
    if (port_active(port.flags)
        && ((port.flags & static_cast<unsigned>(PortFlag::Collecting)) == 0
            || (port.flags & static_cast<unsigned>(PortFlag::Distributing)) == 0)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned proto_table_size() noexcept {
    return 6;
}

} // namespace pbsd::kernel::lagg
