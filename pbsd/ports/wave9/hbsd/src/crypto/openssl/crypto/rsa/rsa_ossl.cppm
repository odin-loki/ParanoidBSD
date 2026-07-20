export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.rsa.rsa_ossl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/rsa/rsa_ossl.c
// void rsa_ossl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/rsa/rsa_ossl.c wave=wave9 loc=1203
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_ossl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_ossl
