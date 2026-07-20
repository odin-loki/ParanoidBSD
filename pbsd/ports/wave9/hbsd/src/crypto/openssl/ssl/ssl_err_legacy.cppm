export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.ssl_err_legacy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/ssl_err_legacy.c
// void ssl_err_legacy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/ssl_err_legacy.c wave=wave9 loc=21
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::ssl_err_legacy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::ssl_err_legacy
