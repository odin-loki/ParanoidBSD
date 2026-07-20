export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_accel.mlx5_ipsec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_accel/mlx5_ipsec.c
// void mlx5_ipsec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_accel/mlx5_ipsec.c wave=wave5 loc=833
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_accel::mlx5_ipsec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_accel::mlx5_ipsec
