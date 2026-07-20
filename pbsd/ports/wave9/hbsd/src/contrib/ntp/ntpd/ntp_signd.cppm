export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_signd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_signd.c
// void ntp_signd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_signd.c wave=wave9 loc=240
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_signd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_signd
