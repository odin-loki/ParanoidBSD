module;
#include <cstdint>

export module pbsd.bifrost.vserial;

import pbsd.core;

/// PROVENANCE: BIFROST HV — paravirtual serial scaffold.
export namespace pbsd::bifrost::vserial {

inline constexpr std::uint32_t kMmioBase = 0xF000'4000u;
inline constexpr unsigned kBaudDefault = 115200;

struct Device {
    std::uint32_t mmio_base{kMmioBase};
    unsigned baud{kBaudDefault};
    bool enabled{false};
};

[[nodiscard]] inline Status attach(Device& d, unsigned baud) noexcept {
    if (baud == 0) {
        return Status::Invalid;
    }
    d.baud = baud;
    d.enabled = true;
    return Status::Ok;
}

} // namespace pbsd::bifrost::vserial
