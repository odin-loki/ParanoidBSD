export module pbsd.port.wave9.hbsd.src.contrib.pam_krb5.module.alt_auth;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/pam-krb5/module/alt-auth.c
// void alt-auth_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/pam-krb5/module/alt-auth.c wave=wave9 loc=240
export namespace pbsd::port::wave9::hbsd::src::contrib::pam_krb5::module::alt_auth {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::pam_krb5::module::alt_auth
