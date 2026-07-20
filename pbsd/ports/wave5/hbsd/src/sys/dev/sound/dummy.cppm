export module pbsd.port.wave5.hbsd.src.sys.dev.sound.dummy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/sound/dummy.c
// void dummy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/sound/dummy.c wave=wave5 loc=438
export namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::dummy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::sound::dummy
