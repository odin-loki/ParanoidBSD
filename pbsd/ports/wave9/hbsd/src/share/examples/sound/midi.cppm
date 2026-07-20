export module pbsd.port.wave9.hbsd.src.share.examples.sound.midi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/share/examples/sound/midi.c
// void midi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/share/examples/sound/midi.c wave=wave9 loc=89
export namespace pbsd::port::wave9::hbsd::src::share::examples::sound::midi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::share::examples::sound::midi
