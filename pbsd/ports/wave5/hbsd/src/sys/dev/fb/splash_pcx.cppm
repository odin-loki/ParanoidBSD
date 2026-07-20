export module pbsd.port.wave5.hbsd.src.sys.dev.fb.splash_pcx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/fb/splash_pcx.c
// void splash_pcx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/fb/splash_pcx.c wave=wave5 loc=269
export namespace pbsd::port::wave5::hbsd::src::sys::dev::fb::splash_pcx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::fb::splash_pcx
