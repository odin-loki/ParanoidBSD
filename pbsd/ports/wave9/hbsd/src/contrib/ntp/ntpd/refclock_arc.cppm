export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_arc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_arc.c
// void refclock_arc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_arc.c wave=wave9 loc=1590
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_arc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_arc
