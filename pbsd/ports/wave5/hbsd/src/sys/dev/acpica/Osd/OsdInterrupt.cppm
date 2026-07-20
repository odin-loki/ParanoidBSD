export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.osd.osdinterrupt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/Osd/OsdInterrupt.c
// void OsdInterrupt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/Osd/OsdInterrupt.c wave=wave5 loc=217
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::osd::osdinterrupt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::osd::osdinterrupt
