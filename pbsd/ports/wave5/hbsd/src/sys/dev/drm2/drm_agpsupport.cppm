export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_agpsupport;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_agpsupport.c
// void drm_agpsupport_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_agpsupport.c wave=wave5 loc=466
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_agpsupport {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_agpsupport
