export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_shm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_shm.c
// void refclock_shm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_shm.c wave=wave9 loc=664
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_shm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_shm
