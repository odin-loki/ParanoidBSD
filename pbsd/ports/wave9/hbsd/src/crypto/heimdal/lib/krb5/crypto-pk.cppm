export module pbsd.port.wave9.hbsd.src.crypto.heimdal.lib.krb5.crypto_pk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/heimdal/lib/krb5/crypto-pk.c
// void crypto-pk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/heimdal/lib/krb5/crypto-pk.c wave=wave9 loc=301
export namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::krb5::crypto_pk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::krb5::crypto_pk
