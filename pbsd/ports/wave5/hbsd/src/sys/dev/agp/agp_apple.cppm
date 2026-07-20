export module pbsd.port.wave5.hbsd.src.sys.dev.agp.agp_apple;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/agp/agp_apple.c
// void agp_apple_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/agp/agp_apple.c wave=wave5 loc=292
export namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_apple {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_apple
