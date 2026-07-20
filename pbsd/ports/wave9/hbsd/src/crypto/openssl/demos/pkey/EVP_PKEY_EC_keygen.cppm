export module pbsd.port.wave9.hbsd.src.crypto.openssl.demos.pkey.evp_pkey_ec_keygen;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/demos/pkey/EVP_PKEY_EC_keygen.c
// void EVP_PKEY_EC_keygen_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/demos/pkey/EVP_PKEY_EC_keygen.c wave=wave9 loc=155
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::demos::pkey::evp_pkey_ec_keygen {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::demos::pkey::evp_pkey_ec_keygen
