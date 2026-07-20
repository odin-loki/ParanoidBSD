module;
#include <cstdint>

export module pbsd.uda.cas;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cas/if_cas.c
export namespace pbsd::uda::cas {

inline constexpr std::uint32_t kRegIntMask = 0x0004;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCasInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cas_generic() noexcept {
    return Descriptor{
        .name = "cas",
        .provenance = "hbsd/src/sys/dev/cas/if_cas.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x108e,
        .device_id = 0x0021,
        .init_sequence = kCasInit,
        .reset_sequence = kCasInit,
    };
}

} // namespace pbsd::uda::cas
