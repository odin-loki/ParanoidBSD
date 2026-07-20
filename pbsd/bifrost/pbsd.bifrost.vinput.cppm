module;
#include <cstdint>

export module pbsd.bifrost.vinput;

import pbsd.core;

/// PROVENANCE: BIFROST HV — paravirtual input scaffold.
export namespace pbsd::bifrost::vinput {

inline constexpr std::uint32_t kMmioBase = 0xF000'3000u;

enum class EventType : unsigned char {
    Key = 0,
    Pointer = 1,
    Touch = 2,
};

struct Device {
    std::uint32_t mmio_base{kMmioBase};
    bool enabled{false};
};

[[nodiscard]] inline Status attach(Device& d) noexcept {
    d.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_event(unsigned type) noexcept {
    return type <= static_cast<unsigned>(EventType::Touch) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::vinput
