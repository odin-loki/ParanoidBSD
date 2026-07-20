export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.tls_depr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/tls_depr.c
// void tls_depr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/tls_depr.c wave=wave9 loc=216
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::tls_depr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::tls_depr
