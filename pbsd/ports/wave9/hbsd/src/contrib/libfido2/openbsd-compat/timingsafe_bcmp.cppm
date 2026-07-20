export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.timingsafe_bcmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/timingsafe_bcmp.c
// void timingsafe_bcmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/timingsafe_bcmp.c wave=wave9 loc=35
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::timingsafe_bcmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::timingsafe_bcmp
