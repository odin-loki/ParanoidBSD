export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_oncore;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_oncore.c
// void refclock_oncore_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_oncore.c wave=wave9 loc=4140
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_oncore {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_oncore
