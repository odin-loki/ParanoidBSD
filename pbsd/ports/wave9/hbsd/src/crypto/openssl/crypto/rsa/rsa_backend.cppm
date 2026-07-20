export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.rsa.rsa_backend;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/rsa/rsa_backend.c
// void rsa_backend_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/rsa/rsa_backend.c wave=wave9 loc=713
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_backend {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_backend
