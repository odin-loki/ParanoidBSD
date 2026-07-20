export module pbsd.port.wave5.hbsd.src.sys.dev.sound.pcm.dsp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/pcm/dsp.c
// void dsp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/pcm/dsp.c wave=wave5 loc=3211
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pcm::dsp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::pcm::dsp
