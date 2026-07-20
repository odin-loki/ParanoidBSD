export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_ib.mlx4_ib_cq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_ib/mlx4_ib_cq.c
// void mlx4_ib_cq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_ib/mlx4_ib_cq.c wave=wave5 loc=971
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_ib::mlx4_ib_cq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_ib::mlx4_ib_cq
