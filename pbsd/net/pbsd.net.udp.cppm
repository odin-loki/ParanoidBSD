module;
#include <cstdint>

export module pbsd.net.udp;

import pbsd.core;
import pbsd.rights;
import pbsd.net.sockopt;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/netinet/udp.h — UDP socket options and ESP-in-UDP constants.
export namespace pbsd::net::udp {

enum class Option : int {
    Encap = 1,
};

enum class EncapType : int {
    EspInUdpNonIke = 1,
    EspInUdp       = 2,
};

inline constexpr std::uint16_t kEspInUdpPort      = 500;
inline constexpr std::uint16_t kEspInUdpMaxFragLen = 552;

struct OptionEntry {
    Option           name{};
    CapabilityRights need_read{CapabilityRights::None};
    CapabilityRights need_write{CapabilityRights::None};
};

inline constexpr OptionEntry kUdpOptionTable[] = {
    {Option::Encap, CapabilityRights::Read, CapabilityRights::Write},
};

[[nodiscard]] inline constexpr std::size_t option_table_size() noexcept {
    return sizeof(kUdpOptionTable) / sizeof(kUdpOptionTable[0]);
}

[[nodiscard]] inline CapabilityRights rights_for(Option opt, sockopt::OptDir dir) noexcept {
    for (auto const& e : kUdpOptionTable) {
        if (e.name != opt) {
            continue;
        }
        if (dir == sockopt::OptDir::Set) {
            return e.need_write;
        }
        return e.need_read;
    }
    return dir == sockopt::OptDir::Set ? CapabilityRights::Write : CapabilityRights::Read;
}

[[nodiscard]] inline Status check_option(CapabilityRights sock_rights, Option opt,
                                         sockopt::OptDir dir) noexcept {
    CapabilityRights need = rights_for(opt, dir);
    if (!has_right(sock_rights, need)) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool valid_encap_type(int val) noexcept {
    return val == static_cast<int>(EncapType::EspInUdpNonIke)
        || val == static_cast<int>(EncapType::EspInUdp);
}

} // namespace pbsd::net::udp
