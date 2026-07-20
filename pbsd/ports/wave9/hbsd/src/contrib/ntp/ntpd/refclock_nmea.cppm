export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_nmea;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_nmea.c
// void refclock_nmea_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_nmea.c wave=wave9 loc=1825
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_nmea {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_nmea
