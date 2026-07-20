module;
#include <cstdint>

export module pbsd.uda.mvneta;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/neta/if_mvneta.c
export namespace pbsd::uda::mvneta {

inline constexpr std::uint32_t kRegPortCfg = 0x0000;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 3 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMvnetaInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mvneta_generic() noexcept {
    return Descriptor{
        .name = "mvneta",
        .provenance = "hbsd/src/sys/dev/neta/if_mvneta.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kMvnetaInit,
        .reset_sequence = kMvnetaInit,
    };
}

} // namespace pbsd::uda::mvneta
