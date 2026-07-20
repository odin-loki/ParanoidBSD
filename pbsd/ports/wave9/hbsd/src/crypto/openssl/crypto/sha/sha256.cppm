export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.sha.sha256;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/sha/sha256.c
// void sha256_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/sha/sha256.c wave=wave9 loc=438
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::sha::sha256 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::sha::sha256
