export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_local;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_local.c
// void refclock_local_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_local.c wave=wave9 loc=217
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_local {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_local
