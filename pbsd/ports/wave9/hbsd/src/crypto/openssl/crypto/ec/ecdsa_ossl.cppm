export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.ec.ecdsa_ossl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/ec/ecdsa_ossl.c
// void ecdsa_ossl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/ec/ecdsa_ossl.c wave=wave9 loc=549
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecdsa_ossl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecdsa_ossl
