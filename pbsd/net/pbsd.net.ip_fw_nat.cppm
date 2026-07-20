module;
#include <cstdint>

export module pbsd.net.ip_fw_nat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netpfil/ipfw/ip_fw_nat.h — ipfw NAT rule flags.
export namespace pbsd::net::ip_fw_nat {

enum class Flag : unsigned {
    In  = 0x0001,
    Out = 0x0002,
    Any = 0x0004,
};

struct Rule {
    unsigned id{};
    unsigned flags{};
    std::uint32_t addr{};
    unsigned short port{};
};

[[nodiscard]] inline Status validate_rule(const Rule& r) noexcept {
    if (r.id == 0) {
        return Status::Invalid;
    }
    if ((r.flags & ~(static_cast<unsigned>(Flag::In) | static_cast<unsigned>(Flag::Out)
                     | static_cast<unsigned>(Flag::Any))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool matches_direction(unsigned flags, bool outbound) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Any)) != 0) {
        return true;
    }
    if (outbound) {
        return (flags & static_cast<unsigned>(Flag::Out)) != 0;
    }
    return (flags & static_cast<unsigned>(Flag::In)) != 0;
}

} // namespace pbsd::net::ip_fw_nat
