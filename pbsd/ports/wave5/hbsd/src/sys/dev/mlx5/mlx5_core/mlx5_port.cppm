export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_core.mlx5_port;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_port.c
// void mlx5_port_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_port.c wave=wave5 loc=1513
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_core::mlx5_port {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_core::mlx5_port
