export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_wwv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_wwv.c
// void refclock_wwv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_wwv.c wave=wave9 loc=2711
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_wwv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_wwv
