export module pbsd.port.wave5.hbsd.src.sys.dev.spibus.ofw_spibus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/spibus/ofw_spibus.c
// void ofw_spibus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/spibus/ofw_spibus.c wave=wave5 loc=244
export namespace pbsd::port::wave5::hbsd::src::sys::dev::spibus::ofw_spibus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::spibus::ofw_spibus
