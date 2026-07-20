export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.threads_pthread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/threads_pthread.c
// void threads_pthread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/threads_pthread.c wave=wave9 loc=958
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::threads_pthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::threads_pthread
