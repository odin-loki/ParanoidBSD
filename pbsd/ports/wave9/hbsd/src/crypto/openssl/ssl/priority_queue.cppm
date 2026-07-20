export module pbsd.port.wave9.hbsd.src.crypto.openssl.ssl.priority_queue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/ssl/priority_queue.c
// void priority_queue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/ssl/priority_queue.c wave=wave9 loc=373
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::priority_queue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::ssl::priority_queue
