export module pbsd.port.wave5.hbsd.src.sys.dev.fb.splash_bmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/fb/splash_bmp.c
// void splash_bmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/fb/splash_bmp.c wave=wave5 loc=601
export namespace pbsd::port::wave5::hbsd::src::sys::dev::fb::splash_bmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::fb::splash_bmp
