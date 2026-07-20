export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_stub;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_stub.c
// void drm_stub_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_stub.c wave=wave5 loc=499
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_stub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_stub
