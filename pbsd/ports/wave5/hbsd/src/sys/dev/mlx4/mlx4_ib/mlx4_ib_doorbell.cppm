export module pbsd.port.wave5.hbsd.src.sys.dev.mlx4.mlx4_ib.mlx4_ib_doorbell;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mlx4/mlx4_ib/mlx4_ib_doorbell.c
// void mlx4_ib_doorbell_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mlx4/mlx4_ib/mlx4_ib_doorbell.c wave=wave5 loc=96
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_ib::mlx4_ib_doorbell {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mlx4::mlx4_ib::mlx4_ib_doorbell
