export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_chroot.pam_chroot;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_chroot/pam_chroot.c
// void pam_chroot_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_chroot/pam_chroot.c wave=wave2 loc=108
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_chroot::pam_chroot {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_chroot::pam_chroot
