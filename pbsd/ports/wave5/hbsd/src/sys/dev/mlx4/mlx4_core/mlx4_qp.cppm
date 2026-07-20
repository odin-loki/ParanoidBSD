export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_core.mlx4_qp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_qp.c
// void mlx4_qp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_qp.c wave=wave5 loc=950
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_qp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_qp
