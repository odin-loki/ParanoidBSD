export module pbsd.port.wave5.hbsd.src.sys.dev.random.ivy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/random/ivy.c
// void ivy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/random/ivy.c wave=wave5 loc=165
export namespace pbsd::port::wave5::hbsd::src::sys::dev::random::ivy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::random::ivy
