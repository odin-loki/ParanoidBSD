module;
#include <cstdint>

export module pbsd.uda.mpi3mr;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mpi3mr/mpi3mr.c
export namespace pbsd::uda::mpi3mr {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_ioc(unsigned ioc) noexcept {
    return ioc < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMpi3mrInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mpi3mr_generic() noexcept {
    return Descriptor{
        .name = "mpi3mr",
        .provenance = "hbsd/src/sys/dev/mpi3mr/mpi3mr.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x00A5,
        .init_sequence = kMpi3mrInit,
        .reset_sequence = kMpi3mrInit,
    };
}

} // namespace pbsd::uda::mpi3mr
