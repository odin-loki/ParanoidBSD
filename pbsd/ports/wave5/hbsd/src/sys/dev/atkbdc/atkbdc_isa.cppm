export module pbsd.port.wave5.hbsd.src.sys.dev.atkbdc.atkbdc_isa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/atkbdc/atkbdc_isa.c
// void atkbdc_isa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/atkbdc/atkbdc_isa.c wave=wave5 loc=321
export namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbdc_isa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::atkbdc::atkbdc_isa
