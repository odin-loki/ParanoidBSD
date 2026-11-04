module;
#include <cstdint>

export module pbsd.uda.mlx5;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mlx5/mlx5_ifc.h, mlx5_core.h
export namespace pbsd::uda::mlx5 {

inline constexpr std::uint32_t kRegHcr = 0x00000000;
inline constexpr std::uint32_t kRegHcrGo = 0x00000001;
inline constexpr std::uint32_t kRegHcrStatus = 0x00000002;

inline constexpr std::uint16_t kPciVendorMellanox = 0x15B3;
inline constexpr std::uint16_t kPciDevConnectX4    = 0x1013;

inline constexpr RegInsn kMlx5Init[] = {
    {RegOp::Write32, kRegHcr, 0, 0, 0},
    {RegOp::CheckEq, kRegHcr, kRegHcrStatus, 0x1, 5000},
    {RegOp::Write32, kRegHcr, kRegHcrGo, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMlx5Reset[] = {
    {RegOp::Write32, kRegHcr, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mlx5_connectx4() noexcept {
    return Descriptor{
        .name = "mlx5-connectx4",
        .provenance = "hbsd/src/sys/dev/mlx5/mlx5_core.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorMellanox,
        .device_id = kPciDevConnectX4,
        .init_sequence = kMlx5Init,
        .reset_sequence = kMlx5Reset,
    };
}

} // namespace pbsd::uda::mlx5
