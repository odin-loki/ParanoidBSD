export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.rsa.rsa_pss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/rsa/rsa_pss.c
// void rsa_pss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/rsa/rsa_pss.c wave=wave9 loc=424
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_pss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::rsa::rsa_pss
