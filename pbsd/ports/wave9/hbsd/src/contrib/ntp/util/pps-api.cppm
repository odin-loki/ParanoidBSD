export module pbsd.port.wave9.hbsd.src.contrib.ntp.util.pps_api;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/util/pps-api.c
// void pps-api_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/util/pps-api.c wave=wave9 loc=100
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::util::pps_api {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::util::pps_api
