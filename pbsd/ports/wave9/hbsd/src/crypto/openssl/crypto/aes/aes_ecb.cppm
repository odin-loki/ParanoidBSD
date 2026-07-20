export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.aes.aes_ecb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/aes/aes_ecb.c
// void aes_ecb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/aes/aes_ecb.c wave=wave9 loc=32
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::aes::aes_ecb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::aes::aes_ecb
