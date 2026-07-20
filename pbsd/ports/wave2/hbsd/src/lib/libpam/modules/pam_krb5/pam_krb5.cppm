export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_krb5.pam_krb5;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_krb5/pam_krb5.c
// void pam_krb5_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_krb5/pam_krb5.c wave=wave2 loc=1076
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_krb5::pam_krb5 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_krb5::pam_krb5
