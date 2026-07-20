export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.osd.osdhardware;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/Osd/OsdHardware.c
// void OsdHardware_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/Osd/OsdHardware.c wave=wave5 loc=140
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::osd::osdhardware {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::osd::osdhardware
