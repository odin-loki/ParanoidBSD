export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_refclock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_refclock.c
// void ntp_refclock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_refclock.c wave=wave9 loc=1852
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_refclock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_refclock
