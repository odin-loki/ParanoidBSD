export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.ttm.ttm_agp_backend;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/ttm/ttm_agp_backend.c
// void ttm_agp_backend_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/ttm/ttm_agp_backend.c wave=wave5 loc=134
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::ttm::ttm_agp_backend {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::ttm::ttm_agp_backend
