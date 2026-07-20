export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_util;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_util.c
// void ntp_util_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_util.c wave=wave9 loc=1118
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_util {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_util
