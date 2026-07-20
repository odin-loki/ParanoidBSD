module;
#include <cstdint>

export module pbsd.uda.isci;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/isci/isci.c
export namespace pbsd::uda::isci {

inline constexpr std::uint32_t kRegSmu = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIsciInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor isci_generic() noexcept {
    return Descriptor{
        .name = "isci",
        .provenance = "hbsd/src/sys/dev/isci/isci.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x8086,
        .device_id = 0x1D60,
        .init_sequence = kIsciInit,
        .reset_sequence = kIsciInit,
    };
}

} // namespace pbsd::uda::isci
