export module pbsd.port.wave5.hbsd.src.sys.dev.atkbdc.atkbdc_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/atkbdc/atkbdc_subr.c
// void atkbdc_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/atkbdc/atkbdc_subr.c wave=wave5 loc=127
export namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbdc_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbdc_subr
