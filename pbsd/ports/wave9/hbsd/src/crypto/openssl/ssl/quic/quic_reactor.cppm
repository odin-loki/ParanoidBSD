export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.quic.quic_reactor;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/quic/quic_reactor.c
// void quic_reactor_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/quic/quic_reactor.c wave=wave9 loc=632
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_reactor {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_reactor
