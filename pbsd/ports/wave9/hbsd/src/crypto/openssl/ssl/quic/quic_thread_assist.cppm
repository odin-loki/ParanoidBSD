export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.quic.quic_thread_assist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/quic/quic_thread_assist.c
// void quic_thread_assist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/quic/quic_thread_assist.c wave=wave9 loc=148
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_thread_assist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_thread_assist
