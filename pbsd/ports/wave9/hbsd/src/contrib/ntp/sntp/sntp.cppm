export module pbsd.port.wave9.hbsd.src.contrib.ntp.sntp.sntp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/sntp/sntp.c
// void sntp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/sntp/sntp.c wave=wave9 loc=14
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::sntp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::sntp
