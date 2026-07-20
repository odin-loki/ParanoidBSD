module;
#include <cstdint>

export module pbsd.uda.gem;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/gem/if_gem.c
export namespace pbsd::uda::gem {

inline constexpr std::uint32_t kRegConfig = 0x0000;
inline constexpr std::uint32_t kConfigReset = 0x80000000u;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kGemInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor gem_generic() noexcept {
    return Descriptor{
        .name = "gem",
        .provenance = "hbsd/src/sys/dev/gem/if_gem.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x108e,
        .device_id = 0x0021,
        .init_sequence = kGemInit,
        .reset_sequence = kGemInit,
    };
}

} // namespace pbsd::uda::gem
