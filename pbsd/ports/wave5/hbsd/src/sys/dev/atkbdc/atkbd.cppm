export module pbsd.port.wave5.hbsd.src.sys.dev.atkbdc.atkbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/atkbdc/atkbd.c
// void atkbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/atkbdc/atkbd.c wave=wave5 loc=1613
export namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbd
