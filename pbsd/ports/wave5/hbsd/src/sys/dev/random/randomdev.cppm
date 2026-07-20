export module pbsd.port.wave5.hbsd.src.sys.dev.random.randomdev;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/random/randomdev.c
// void randomdev_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/random/randomdev.c wave=wave5 loc=429
export namespace pbsd::port::wave5::hbsd::src::sys::dev::random::randomdev {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::random::randomdev
