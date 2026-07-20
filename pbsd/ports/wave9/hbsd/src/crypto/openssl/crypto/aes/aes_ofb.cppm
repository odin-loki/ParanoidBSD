export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.aes.aes_ofb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/aes/aes_ofb.c
// void aes_ofb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/aes/aes_ofb.c wave=wave9 loc=25
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::aes::aes_ofb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::aes::aes_ofb
