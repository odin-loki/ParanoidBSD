module;
#include <cstdint>

export module pbsd.uda.mgb;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mgb/if_mgb.c
export namespace pbsd::uda::mgb {

inline constexpr std::uint32_t kRegMacCtl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMgbInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mgb_generic() noexcept {
    return Descriptor{
        .name = "mgb",
        .provenance = "hbsd/src/sys/dev/mgb/if_mgb.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x15ad,
        .device_id = 0x1533,
        .init_sequence = kMgbInit,
        .reset_sequence = kMgbInit,
    };
}

} // namespace pbsd::uda::mgb
