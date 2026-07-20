export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_ib.mlx5_ib_gsi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_ib/mlx5_ib_gsi.c
// void mlx5_ib_gsi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_ib/mlx5_ib_gsi.c wave=wave5 loc=537
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_ib::mlx5_ib_gsi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_ib::mlx5_ib_gsi
