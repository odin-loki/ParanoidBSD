export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl_aes_gcm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl_aes_gcm.c
// void ossl_aes_gcm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl_aes_gcm.c wave=wave4 loc=615
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aes_gcm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aes_gcm
