module;
#include <cstdint>

export module pbsd.bifrost.vblk;

import pbsd.core;

/// PROVENANCE: BIFROST HV — paravirtual block scaffold.
export namespace pbsd::bifrost::vblk {

inline constexpr std::uint32_t kMmioBase = 0xF000'0000u;
inline constexpr unsigned kSectorSize = 512;

struct Device {
    std::uint32_t mmio_base{kMmioBase};
    std::uint64_t capacity_sectors{};
    bool enabled{false};
};

[[nodiscard]] inline Status attach(Device& d, std::uint64_t sectors) noexcept {
    if (sectors == 0) {
        return Status::Invalid;
    }
    d.capacity_sectors = sectors;
    d.enabled = true;
    return Status::Ok;
}

} // namespace pbsd::bifrost::vblk
