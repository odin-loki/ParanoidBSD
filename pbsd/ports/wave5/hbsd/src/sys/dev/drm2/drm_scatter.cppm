export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_scatter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_scatter.c
// void drm_scatter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_scatter.c wave=wave5 loc=134
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_scatter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_scatter
