export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl_arm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl_arm.c
// void ossl_arm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl_arm.c wave=wave4 loc=62
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_arm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl_arm
