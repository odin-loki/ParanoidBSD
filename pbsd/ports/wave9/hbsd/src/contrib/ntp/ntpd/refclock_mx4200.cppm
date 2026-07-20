export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_mx4200;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_mx4200.c
// void refclock_mx4200_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_mx4200.c wave=wave9 loc=1639
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_mx4200 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_mx4200
