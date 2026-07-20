export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_zyfer;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_zyfer.c
// void refclock_zyfer_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_zyfer.c wave=wave9 loc=348
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_zyfer {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_zyfer
