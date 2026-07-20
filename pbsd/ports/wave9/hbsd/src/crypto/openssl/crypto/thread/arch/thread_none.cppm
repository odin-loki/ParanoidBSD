export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.thread.arch.thread_none;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/thread/arch/thread_none.c
// void thread_none_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/thread/arch/thread_none.c wave=wave9 loc=82
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::thread::arch::thread_none {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::thread::arch::thread_none
