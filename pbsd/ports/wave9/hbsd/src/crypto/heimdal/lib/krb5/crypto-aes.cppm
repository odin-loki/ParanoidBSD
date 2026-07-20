export module pbsd.port.wave9.hbsd.src.crypto.heimdal.lib.krb5.crypto_aes;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/heimdal/lib/krb5/crypto-aes.c
// void crypto-aes_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/heimdal/lib/krb5/crypto-aes.c wave=wave9 loc=172
export namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::krb5::crypto_aes {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::krb5::crypto_aes
