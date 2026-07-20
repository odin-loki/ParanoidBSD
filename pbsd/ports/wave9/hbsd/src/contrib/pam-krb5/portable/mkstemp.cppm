export module pbsd.port.wave9.hbsd.src.contrib.pam_krb5.portable.mkstemp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/pam-krb5/portable/mkstemp.c
// void mkstemp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/pam-krb5/portable/mkstemp.c wave=wave9 loc=101
export namespace pbsd::port::wave9::hbsd::src::contrib::pam_krb5::portable::mkstemp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::pam_krb5::portable::mkstemp
