module;
#include <cstdint>

export module pbsd.bifrost.vconsole;

import pbsd.core;

/// PROVENANCE: BIFROST HV — paravirtual console scaffold.
export namespace pbsd::bifrost::vconsole {

inline constexpr std::uint32_t kMmioBase = 0xF000'2000u;

struct Device {
    std::uint32_t mmio_base{kMmioBase};
    bool enabled{false};
};

[[nodiscard]] inline Status attach(Device& d) noexcept {
    d.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline Status detach(Device& d) noexcept {
    d.enabled = false;
    return Status::Ok;
}

} // namespace pbsd::bifrost::vconsole
