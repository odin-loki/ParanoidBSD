export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_jupiter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_jupiter.c
// void refclock_jupiter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_jupiter.c wave=wave9 loc=1032
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_jupiter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_jupiter
