export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.ec.ecdh_ossl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/ec/ecdh_ossl.c
// void ecdh_ossl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/ec/ecdh_ossl.c wave=wave9 loc=147
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecdh_ossl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecdh_ossl
