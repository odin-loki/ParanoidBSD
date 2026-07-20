export module pbsd.port.wave5.hbsd.src.sys.dev.speaker.spkr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/speaker/spkr.c
// void spkr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/speaker/spkr.c wave=wave5 loc=521
export namespace pbsd::port::wave5::hbsd::src::sys::dev::speaker::spkr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::speaker::spkr
