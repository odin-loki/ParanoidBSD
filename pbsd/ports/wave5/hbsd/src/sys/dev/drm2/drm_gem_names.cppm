export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_gem_names;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_gem_names.c
// void drm_gem_names_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_gem_names.c wave=wave5 loc=227
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_gem_names {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_gem_names
