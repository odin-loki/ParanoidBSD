export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_neoclock4x;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_neoclock4x.c
// void refclock_neoclock4x_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_neoclock4x.c wave=wave9 loc=1124
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_neoclock4x {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_neoclock4x
