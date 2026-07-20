export module pbsd.port.wave5.hbsd.src.sys.dev.sound.pci.hdsp_pcm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/pci/hdsp-pcm.c
// void hdsp-pcm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/pci/hdsp-pcm.c wave=wave5 loc=1132
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdsp_pcm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pci::hdsp_pcm
