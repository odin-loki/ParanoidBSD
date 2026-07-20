export module pbsd.port.wave9.hbsd.src.crypto.openssh.gss_serv_krb5;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/gss-serv-krb5.c
// void gss-serv-krb5_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/gss-serv-krb5.c wave=wave9 loc=211
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::gss_serv_krb5 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::gss_serv_krb5
