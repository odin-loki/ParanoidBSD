export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.quic.quic_record_tx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/quic/quic_record_tx.c
// void quic_record_tx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/quic/quic_record_tx.c wave=wave9 loc=1127
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_record_tx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::quic::quic_record_tx
