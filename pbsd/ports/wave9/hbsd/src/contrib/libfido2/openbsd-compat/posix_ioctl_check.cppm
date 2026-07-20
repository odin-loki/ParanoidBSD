export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.posix_ioctl_check;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/posix_ioctl_check.c
// void posix_ioctl_check_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/posix_ioctl_check.c wave=wave9 loc=7
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::posix_ioctl_check {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::posix_ioctl_check
