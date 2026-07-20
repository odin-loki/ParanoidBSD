export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.rsa.rsa_sign;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/rsa/rsa_sign.c
// void rsa_sign_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/rsa/rsa_sign.c wave=wave9 loc=468
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_sign {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_sign
