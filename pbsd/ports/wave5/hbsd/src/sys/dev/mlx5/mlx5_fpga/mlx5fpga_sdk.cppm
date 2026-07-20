export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_fpga.mlx5fpga_sdk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_fpga/mlx5fpga_sdk.c
// void mlx5fpga_sdk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_fpga/mlx5fpga_sdk.c wave=wave5 loc=541
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_fpga::mlx5fpga_sdk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_fpga::mlx5fpga_sdk
