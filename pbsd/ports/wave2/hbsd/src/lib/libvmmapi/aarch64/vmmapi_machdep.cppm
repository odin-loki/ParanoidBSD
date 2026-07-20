export module pbsd.port.wave2.hbsd.src.lib.libvmmapi.aarch64.vmmapi_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libvmmapi/aarch64/vmmapi_machdep.c
// void vmmapi_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libvmmapi/aarch64/vmmapi_machdep.c wave=wave2 loc=127
export namespace pbsd::port::wave2::hbsd::src::lib::libvmmapi::aarch64::vmmapi_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libvmmapi::aarch64::vmmapi_machdep
