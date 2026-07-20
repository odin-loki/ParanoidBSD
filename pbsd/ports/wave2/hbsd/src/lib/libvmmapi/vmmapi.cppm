export module pbsd.port.wave2.hbsd.src.lib.libvmmapi.vmmapi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libvmmapi/vmmapi.c
// void vmmapi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libvmmapi/vmmapi.c wave=wave2 loc=1319
export namespace pbsd::port::wave2::hbsd::src::lib::libvmmapi::vmmapi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libvmmapi::vmmapi
