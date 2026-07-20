export module pbsd.port.wave5.hbsd.src.sys.dev.sound.pci.hdsp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/pci/hdsp.c
// void hdsp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/pci/hdsp.c wave=wave5 loc=1021
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdsp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdsp
