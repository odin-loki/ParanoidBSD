module;
#include <cstdint>

export module pbsd.uda.ix;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ixgbe/if_ix.c
export namespace pbsd::uda::ix {

inline constexpr std::uint32_t kRegCtrl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 128 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIxInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ix_generic() noexcept {
    return Descriptor{
        .name = "ix",
        .provenance = "hbsd/src/sys/dev/ixgbe/if_ix.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x10fb,
        .init_sequence = kIxInit,
        .reset_sequence = kIxInit,
    };
}

} // namespace pbsd::uda::ix
