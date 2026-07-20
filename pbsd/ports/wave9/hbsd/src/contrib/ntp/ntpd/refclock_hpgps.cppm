export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_hpgps;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_hpgps.c
// void refclock_hpgps_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_hpgps.c wave=wave9 loc=626
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hpgps {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_hpgps
