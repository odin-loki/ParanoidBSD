export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_core.mlx5_vport;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_vport.c
// void mlx5_vport_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_vport.c wave=wave5 loc=1837
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_core::mlx5_vport {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_core::mlx5_vport
