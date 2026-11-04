module;
#include <cstdint>

export module pbsd.uda.pvscsi;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vmware/pvscsi/pvscsi.c
export namespace pbsd::uda::pvscsi {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kPvscsiInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor pvscsi_generic() noexcept {
    return Descriptor{
        .name = "pvscsi",
        .provenance = "hbsd/src/sys/dev/vmware/pvscsi/pvscsi.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x15ad,
        .device_id = 0x07C0,
        .init_sequence = kPvscsiInit,
        .reset_sequence = kPvscsiInit,
    };
}

} // namespace pbsd::uda::pvscsi
