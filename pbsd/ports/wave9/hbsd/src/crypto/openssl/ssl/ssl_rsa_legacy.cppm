export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.ssl_rsa_legacy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/ssl_rsa_legacy.c
// void ssl_rsa_legacy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/ssl_rsa_legacy.c wave=wave9 loc=198
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::ssl_rsa_legacy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::ssl_rsa_legacy
