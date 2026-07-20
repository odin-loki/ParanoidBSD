export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_gem;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_gem.c
// void drm_gem_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_gem.c wave=wave5 loc=495
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_gem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_gem
