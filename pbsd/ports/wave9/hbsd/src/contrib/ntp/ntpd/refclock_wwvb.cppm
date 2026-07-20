export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_wwvb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_wwvb.c
// void refclock_wwvb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_wwvb.c wave=wave9 loc=603
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_wwvb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_wwvb
