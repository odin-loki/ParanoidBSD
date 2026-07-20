export module pbsd.port.wave5.hbsd.src.sys.dev.mlx5.mlx5_ib.mlx5_ib_srq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx5/mlx5_ib/mlx5_ib_srq.c
// void mlx5_ib_srq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx5/mlx5_ib/mlx5_ib_srq.c wave=wave5 loc=496
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_ib::mlx5_ib_srq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx5::mlx5_ib::mlx5_ib_srq
