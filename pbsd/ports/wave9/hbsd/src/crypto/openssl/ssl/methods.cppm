export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.methods;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/methods.c
// void methods_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/methods.c wave=wave9 loc=279
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::methods {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::methods
