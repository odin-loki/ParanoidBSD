export module pbsd.port.wave5.hbsd.src.sys.dev.sound.pci.hdspe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/pci/hdspe.c
// void hdspe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/pci/hdspe.c wave=wave5 loc=913
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdspe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdspe
