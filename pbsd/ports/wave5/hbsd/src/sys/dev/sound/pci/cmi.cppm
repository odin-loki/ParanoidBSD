export module pbsd.port.wave5.hbsd.src.sys.dev.sound.pci.cmi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/pci/cmi.c
// void cmi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/pci/cmi.c wave=wave5 loc=1110
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::cmi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::cmi
