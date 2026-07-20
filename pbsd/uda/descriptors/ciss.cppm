module;
#include <cstdint>

export module pbsd.uda.ciss;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ciss/ciss.c
export namespace pbsd::uda::ciss {

inline constexpr std::uint32_t kRegInbound = 0x0000;

[[nodiscard]] inline Status validate_lun(unsigned lun) noexcept {
    return lun < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCissInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ciss_generic() noexcept {
    return Descriptor{
        .name = "ciss",
        .provenance = "hbsd/src/sys/dev/ciss/ciss.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x103c,
        .device_id = 0x3239,
        .init_sequence = kCissInit,
        .reset_sequence = kCissInit,
    };
}

} // namespace pbsd::uda::ciss
