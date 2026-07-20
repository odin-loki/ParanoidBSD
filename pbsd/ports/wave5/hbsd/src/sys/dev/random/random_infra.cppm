export module pbsd.port.wave5.hbsd.src.sys.dev.random.random_infra;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/random/random_infra.c
// void random_infra_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/random/random_infra.c wave=wave5 loc=102
export namespace pbsd::port::wave5::hbsd::src::sys::dev::random::random_infra {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::random::random_infra
