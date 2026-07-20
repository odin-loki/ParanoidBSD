export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.siphash.siphash;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/siphash/siphash.c
// void siphash_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/siphash/siphash.c wave=wave9 loc=252
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::siphash::siphash {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::siphash::siphash
