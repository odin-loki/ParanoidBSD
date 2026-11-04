module;
#include <cstdint>

export module pbsd.net.radix;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/radix.h — radix route tree node flags.
export namespace pbsd::net::radix {

enum class NodeFlag : unsigned char {
    Normal = 0x01,
    Root   = 0x02,
    Active = 0x04,
};

[[nodiscard]] inline bool is_leaf(unsigned char flags) noexcept {
    return (flags & static_cast<unsigned char>(NodeFlag::Normal)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned char flags) noexcept {
    if ((flags & static_cast<unsigned char>(NodeFlag::Root))
        && !(flags & static_cast<unsigned char>(NodeFlag::Normal))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::radix
