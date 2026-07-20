export module pbsd.port.wave4.hbsd.src.sys.crypto.openssl.ossl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/openssl/ossl.c
// void ossl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/openssl/ossl.c wave=wave4 loc=484
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::openssl::ossl
