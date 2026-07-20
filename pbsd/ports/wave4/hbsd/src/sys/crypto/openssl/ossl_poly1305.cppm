export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl_poly1305;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl_poly1305.c
// void ossl_poly1305_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl_poly1305.c wave=wave4 loc=181
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_poly1305 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_poly1305
