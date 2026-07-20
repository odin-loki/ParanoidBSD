export module pbsd.rights.mac;

import pbsd.core;
import pbsd.rights;

/// MAC-specific capability rights (SI-1 subset checks).
export namespace pbsd::rights::mac {

enum class MacRights : unsigned int {
    None = 0,
    LabelRead = 1u << 0,
    LabelWrite = 1u << 1,
    PolicyLoad = 1u << 2,
    All = 0x07u,
};

[[nodiscard]] constexpr MacRights operator|(MacRights a, MacRights b) noexcept {
    return static_cast<MacRights>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr MacRights operator&(MacRights a, MacRights b) noexcept {
    return static_cast<MacRights>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool mac_subset(MacRights child, MacRights parent) noexcept {
    return (static_cast<unsigned>(child) & ~static_cast<unsigned>(parent)) == 0u;
}

[[nodiscard]] constexpr Status check_mac_grant(MacRights parent, MacRights child) noexcept {
    return mac_subset(child, parent) ? Status::Ok : Status::Denied;
}

} // namespace pbsd::rights::mac
