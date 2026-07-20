module;
#include <cstdint>

export module pbsd.bifrost.vballoon;

import pbsd.core;

/// PROVENANCE: BIFROST HV — paravirtual memory balloon scaffold.
export namespace pbsd::bifrost::vballoon {

inline constexpr std::uint32_t kMmioBase = 0xF000'5000u;

struct Device {
    std::uint32_t mmio_base{kMmioBase};
    std::uint64_t target_pages{};
    bool enabled{false};
};

[[nodiscard]] inline Status attach(Device& d, std::uint64_t pages) noexcept {
    d.target_pages = pages;
    d.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline Status deflate(Device& d, std::uint64_t pages) noexcept {
    if (pages > d.target_pages) {
        return Status::Invalid;
    }
    d.target_pages -= pages;
    return Status::Ok;
}

} // namespace pbsd::bifrost::vballoon
