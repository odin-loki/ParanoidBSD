export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.refclock_atom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/refclock_atom.c
// void refclock_atom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/refclock_atom.c wave=wave9 loc=239
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_atom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::refclock_atom
