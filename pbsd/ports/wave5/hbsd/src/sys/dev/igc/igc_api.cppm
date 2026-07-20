export module pbsd.port.wave5.hbsd.src.sys.dev.igc.igc_api;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/igc/igc_api.c
// void igc_api_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/igc/igc_api.c wave=wave5 loc=720
export namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::igc_api {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::igc_api
