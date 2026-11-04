module;
#include <cstdint>

export module pbsd.uda.nicvf;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vnic/nicvf_main.c
export namespace pbsd::uda::nicvf {

inline constexpr std::uint32_t kRegCfg = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_qset(unsigned qset) noexcept {
    return qset < 128 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kNicvfInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nicvf_generic() noexcept {
    return Descriptor{
        .name = "nicvf",
        .provenance = "hbsd/src/sys/dev/vnic/nicvf_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x177d,
        .device_id = 0xA200,
        .init_sequence = kNicvfInit,
        .reset_sequence = kNicvfInit,
    };
}

} // namespace pbsd::uda::nicvf
