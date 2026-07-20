export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_core.mlx4_icm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_icm.c
// void mlx4_icm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_icm.c wave=wave5 loc=463
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_icm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_icm
