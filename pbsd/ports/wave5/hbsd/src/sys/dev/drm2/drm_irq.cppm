export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_irq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/drm2/drm_irq.c
// void drm_irq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_irq.c wave=wave5 loc=1402
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_irq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_irq
