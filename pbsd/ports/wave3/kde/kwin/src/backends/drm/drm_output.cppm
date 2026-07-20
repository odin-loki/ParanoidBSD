export module pbsd.port.wave3.kde.kwin.src.backends.drm.drm_output;

module;
extern "C" {
// Legacy TU symbols — hand-port from kde/kwin/src/backends/drm/drm_output.cpp
// void drm_output_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=kde/kwin/src/backends/drm/drm_output.cpp wave=wave3 loc=822
export namespace pbsd::port::wave3::kde::kwin::src::backends::drm::drm_output {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave3::kde::kwin::src::backends::drm::drm_output
