export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl_aes;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl_aes.c
// void ossl_aes_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl_aes.c wave=wave4 loc=266
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aes {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aes
