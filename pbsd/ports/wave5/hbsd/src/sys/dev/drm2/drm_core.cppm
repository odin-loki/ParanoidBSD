export module pbsd.port.wave5.hbsd.src.sys.dev.drm2.drm_core;

module;
// Header bridge — replace #include of hbsd/src/sys/dev/drm2/drm_core.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/drm2/drm_core.h wave=wave5 loc=36
export namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_core {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::drm2::drm_core
