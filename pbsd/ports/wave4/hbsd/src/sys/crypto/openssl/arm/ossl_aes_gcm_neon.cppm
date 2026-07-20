export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.arm.ossl_aes_gcm_neon;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/arm/ossl_aes_gcm_neon.c
// void ossl_aes_gcm_neon_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/arm/ossl_aes_gcm_neon.c wave=wave4 loc=359
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::arm::ossl_aes_gcm_neon {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::arm::ossl_aes_gcm_neon
