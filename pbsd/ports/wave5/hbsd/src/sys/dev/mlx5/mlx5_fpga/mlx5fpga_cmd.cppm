export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_fpga.mlx5fpga_cmd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_fpga/mlx5fpga_cmd.c
// void mlx5fpga_cmd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_fpga/mlx5fpga_cmd.c wave=wave5 loc=347
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_fpga::mlx5fpga_cmd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_fpga::mlx5fpga_cmd
