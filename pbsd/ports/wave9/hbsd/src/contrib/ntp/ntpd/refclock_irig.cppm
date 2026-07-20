export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_irig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_irig.c
// void refclock_irig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_irig.c wave=wave9 loc=1043
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_irig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_irig
