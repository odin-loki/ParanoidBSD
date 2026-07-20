export module pbsd.port.wave5.hbsd.src.sys.dev.random.darn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/random/darn.c
// void darn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/random/darn.c wave=wave5 loc=145
export namespace pbsd::port::wave5::hbsd::src::sys::dev::random::darn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::random::darn
