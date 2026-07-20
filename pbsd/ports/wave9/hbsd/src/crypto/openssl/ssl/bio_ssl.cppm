export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.bio_ssl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/bio_ssl.c
// void bio_ssl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/bio_ssl.c wave=wave9 loc=541
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::bio_ssl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::bio_ssl
