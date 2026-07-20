export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_core.mlx4_mr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_mr.c
// void mlx4_mr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_mr.c wave=wave5 loc=1161
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_mr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_mr
