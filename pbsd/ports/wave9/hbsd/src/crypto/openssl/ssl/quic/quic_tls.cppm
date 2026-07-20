export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.quic.quic_tls;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/quic/quic_tls.c
// void quic_tls_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/quic/quic_tls.c wave=wave9 loc=951
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_tls {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_tls
