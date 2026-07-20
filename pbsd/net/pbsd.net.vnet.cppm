module;
#include <cstdint>

export module pbsd.net.vnet;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/vnet.h — virtual network stack identifiers.
export namespace pbsd::net::vnet {

inline constexpr unsigned kDefaultId = 0;
inline constexpr unsigned kMaxId = 255;

struct Id {
    unsigned value{kDefaultId};
};

[[nodiscard]] inline Status validate_id(unsigned id) noexcept {
    if (id > kMaxId) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate(const Id& id) noexcept {
    return validate_id(id.value);
}

} // namespace pbsd::net::vnet
