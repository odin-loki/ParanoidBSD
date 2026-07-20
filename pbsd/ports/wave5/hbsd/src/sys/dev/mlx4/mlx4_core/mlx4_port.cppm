export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_core.mlx4_port;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_port.c
// void mlx4_port_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_port.c wave=wave5 loc=2015
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_port {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_core::mlx4_port
