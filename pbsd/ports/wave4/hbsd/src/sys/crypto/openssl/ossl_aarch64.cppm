export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl_aarch64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl_aarch64.c
// void ossl_aarch64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl_aarch64.c wave=wave4 loc=81
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aarch64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_aarch64
